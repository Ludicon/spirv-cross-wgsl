/*
 * Copyright 2026 Ignacio Castano
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * At your option, you may choose to accept this material under either:
 *  1. The Apache License, Version 2.0, found at <http://www.apache.org/licenses/LICENSE-2.0>, or
 *  2. The MIT License, found at <http://opensource.org/licenses/MIT>.
 */

#include "spirv_wgsl.hpp"
#include "GLSL.std.450.h"
#include <algorithm>
#include <assert.h>
#include <functional>
#include <cmath>
#include <stdio.h>
#include <string.h>

using namespace SPIRV_CROSS_SPV_HEADER_NAMESPACE;
using namespace SPIRV_CROSS_NAMESPACE;
using namespace std;

static inline uint32_t round_up(uint32_t value, uint32_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

static const char *const swizzle_components = "xyzw";

string CompilerWGSL::compile()
{
	ir.fixup_reserved_names();

	// WGSL is close enough to modern Vulkan GLSL that we can reuse most of the GLSL code paths.
	options.es = false;
	options.version = 450;
	options.vulkan_semantics = true;
	options.flatten_multidimensional_arrays = false;
	options.force_zero_initialized_variables = false;

	backend.float_literal_suffix = true;
	backend.double_literal_suffix = false;
	backend.long_long_literal_suffix = false;
	backend.uint32_t_literal_suffix = true;
	backend.int16_t_literal_suffix = "";
	backend.uint16_t_literal_suffix = "u";
	backend.basic_int_type = "i32";
	backend.basic_uint_type = "u32";
	backend.discard_literal = "discard";
	backend.demote_literal = "discard";
	backend.boolean_mix_function = "";
	backend.nonuniform_qualifier = "";
	backend.swizzle_is_function = false;
	backend.shared_is_implied = false;
	backend.unsized_array_supported = true;
	backend.explicit_struct_type = false;
	backend.use_initializer_list = false;
	backend.use_typed_initializer_list = false;
	backend.can_declare_struct_inline = true;
	backend.can_declare_arrays_inline = true;
	backend.native_row_major_matrix = false;
	backend.use_constructor_splatting = true;
	backend.allow_precision_qualifiers = false;
	backend.can_swizzle_scalar = false;
	backend.can_return_array = true;
	backend.array_is_value_type = true;
	backend.array_is_value_type_in_buffer_blocks = true;
	backend.comparison_image_samples_scalar = true;
	backend.support_case_fallthrough = false;
	backend.use_array_constructor = false;
	backend.support_precise_qualifier = false;
	backend.supports_extensions = false;
	backend.supports_empty_struct = false;
	backend.support_64bit_switch = false;
	backend.infinite_loop_header = "loop";
	backend.support_do_while = false;
	backend.merge_case_labels = true;
	backend.switch_requires_default = true;
	backend.support_complex_for_loop = false;
	backend.int32_min_literal = "i32(-2147483648)";
	backend.strip_condition_parentheses = true;
	// Switch cases never fall through in WGSL, reaching the end of a case exits the switch.
	backend.unreachable_requires_switch_break = false;

	auto &execution = get_entry_point();
	if (execution.model != ExecutionModelVertex && execution.model != ExecutionModelFragment &&
	    execution.model != ExecutionModelGLCompute)
	{
		SPIRV_CROSS_THROW("WGSL only supports vertex, fragment and compute shaders.");
	}

	// WGSL does not support function overloading, so make sure all function names are unique.
	// In library modules, exported functions claim their names first so that they are preserved.
	{
		unordered_set<string> function_names;
		if (ir.is_library_module)
			for (auto export_id : ir.library_exported_functions)
				function_names.insert(to_name(export_id));

		ir.for_each_typed_id<SPIRFunction>(
		    [&](uint32_t id, SPIRFunction &)
		    {
			    if (ir.is_library_module ? is_library_export(id) : id == ir.default_entry_point)
				    return;
			    auto name = to_name(id);
			    if (function_names.count(name))
			    {
				    name = join(name, "_", id);
				    set_name(id, name);
			    }
			    function_names.insert(name);
		    });
	}

	// Unlike GLSL, struct declarations carry the buffer layout in WGSL, so structurally identical structs
	// with different layouts (e.g. a std140 struct and its function-local copy) cannot be merged.
	// The parser aliases such structs, undo that here, and do not call fixup_type_alias().
	ir.for_each_typed_id<SPIRType>([&](uint32_t, SPIRType &type) { type.type_alias = 0; });
	fixup_anonymous_struct_names();
	build_function_control_flow_graphs_and_analyze();
	update_active_builtins();
	analyze_image_and_sampler_usage();
	analyze_atomics();
	if (wgsl_options.resolve_binding_conflicts)
		resolve_binding_conflicts();
	analyze_mutable_temporaries();
	analyze_single_store_variables();
	analyze_transient_16bit_integers();
	prepare_buffer_layouts();

	uint32_t pass_count = 0;
	do
	{
		reset(pass_count);

		// Move constructor for this type is broken on GCC 4.9 ...
		buffer.reset();

		pointer_parameters.clear();
		warnings.clear();

		emit_header();
		emit_resources();

		if (ir.is_library_module)
		{
			// Emit each exported function as a free function. emit_function recursively emits callees,
			// so internal helpers are picked up too.
			for (auto export_id : ir.library_exported_functions)
				emit_function(get<SPIRFunction>(export_id), Bitset());
		}
		else
		{
			emit_function(get<SPIRFunction>(ir.default_entry_point), Bitset());
			emit_entry_point_wrapper();
		}

		pass_count++;
	} while (is_forcing_recompilation());

	return buffer.str();
}

void CompilerWGSL::require_enable(bool &flag)
{
	if (!flag)
	{
		flag = true;
		force_recompile();
	}
}

bool CompilerWGSL::is_library_export(uint32_t func_id) const
{
	auto &exports = ir.library_exported_functions;
	return find(exports.begin(), exports.end(), FunctionID(func_id)) != exports.end();
}

bool CompilerWGSL::implicit_lod_allowed() const
{
	// Library functions may be called from any stage. WGSL only enforces the fragment stage restriction of
	// implicit LOD sampling for functions reachable from an entry point, so keep the source semantics.
	return ir.is_library_module || get_entry_point().model == ExecutionModelFragment;
}

string CompilerWGSL::get_entry_point_wrapper_name() const
{
	auto &name = get_entry_point().name;
	bool valid =
	    !name.empty() && !isdigit(static_cast<unsigned char>(name[0])) && name != "_" && name.compare(0, 2, "__") != 0;
	for (auto c : name)
		if (!isalnum(static_cast<unsigned char>(c)) && c != '_')
			valid = false;
	return valid ? name : "main";
}

static const char *const warning_names[WGSL_WARNING_COUNT] = {
	"non-finite-constant",
	"depth-compare-lod",
	"ignored-bias",
	"weak-compare-exchange",
	"ignored-builtin",
};

const char *CompilerWGSL::get_warning_name(WGSLWarning kind)
{
	return kind < WGSL_WARNING_COUNT ? warning_names[kind] : "unknown";
}

bool CompilerWGSL::get_warning_from_name(const string &name, WGSLWarning &kind)
{
	for (uint32_t i = 0; i < WGSL_WARNING_COUNT; i++)
	{
		if (name == warning_names[i])
		{
			kind = WGSLWarning(i);
			return true;
		}
	}
	return false;
}

void CompilerWGSL::set_warning_enabled(WGSLWarning kind, bool enabled)
{
	if (enabled)
		disabled_warnings &= ~(1u << kind);
	else
		disabled_warnings |= 1u << kind;
}

bool CompilerWGSL::is_warning_enabled(WGSLWarning kind) const
{
	return (disabled_warnings & (1u << kind)) == 0;
}

string CompilerWGSL::warning_location()
{
	if (!current_function)
		return "at module scope";
	if (!ir.is_library_module && current_function->self == ir.default_entry_point)
		return join("in function '", get_inner_entry_point_name(), "'");
	return join("in function '", to_name(current_function->self), "'");
}

void CompilerWGSL::warn(WGSLWarning kind, const string &message)
{
	if (!is_warning_enabled(kind))
		return;

	// The same construct may be visited several times, e.g. once for every use of a constant.
	for (auto &w : warnings)
		if (w.kind == kind && w.message == message)
			return;

	warnings.push_back({ kind, message });
}

string CompilerWGSL::get_inner_entry_point_name() const
{
	auto &execution = get_entry_point();
	switch (execution.model)
	{
	case ExecutionModelVertex:
		return "vert_main";
	case ExecutionModelFragment:
		return "frag_main";
	default:
		return "comp_main";
	}
}

void CompilerWGSL::emit_header()
{
	bool emitted = false;
	if (requires_f16)
	{
		statement("enable f16;");
		emitted = true;
	}
	if (requires_clip_distances)
	{
		statement("enable clip_distances;");
		emitted = true;
	}
	if (requires_dual_source_blending)
	{
		statement("enable dual_source_blending;");
		emitted = true;
	}
	if (requires_subgroups)
	{
		statement("enable subgroups;");
		emitted = true;
	}
	if (uses_implicit_derivatives && wgsl_options.allow_non_uniform_derivatives)
	{
		statement("diagnostic(off, derivative_uniformity);");
		emitted = true;
	}
	if (emitted)
		statement("");

	emit_helper_functions();
}

void CompilerWGSL::emit_helper_functions()
{
	if (requires_non_finite_helper)
	{
		// WGSL rejects infinity and NaN in constant expressions, but function calls are evaluated at runtime.
		statement("fn spvNonFinite(bits : u32) -> f32");
		begin_scope();
		statement("return bitcast<f32>(bits);");
		end_scope();
		statement("");
	}

	if (requires_inverse_2x2)
	{
		statement("fn spvInverse2x2(m : mat2x2f) -> mat2x2f");
		begin_scope();
		statement("let det = m[0][0] * m[1][1] - m[0][1] * m[1][0];");
		statement("return mat2x2f(m[1][1], -m[0][1], -m[1][0], m[0][0]) * (1.0f / det);");
		end_scope();
		statement("");
	}

	if (requires_inverse_3x3)
	{
		statement("fn spvInverse3x3(m : mat3x3f) -> mat3x3f");
		begin_scope();
		statement("let c0 = cross(m[1], m[2]);");
		statement("let c1 = cross(m[2], m[0]);");
		statement("let c2 = cross(m[0], m[1]);");
		statement("let det = dot(m[0], c0);");
		statement("return transpose(mat3x3f(c0, c1, c2)) * (1.0f / det);");
		end_scope();
		statement("");
	}

	if (requires_inverse_4x4)
	{
		statement("fn spvInverse4x4(m : mat4x4f) -> mat4x4f");
		begin_scope();
		statement("let a00 = m[0][0]; let a01 = m[0][1]; let a02 = m[0][2]; let a03 = m[0][3];");
		statement("let a10 = m[1][0]; let a11 = m[1][1]; let a12 = m[1][2]; let a13 = m[1][3];");
		statement("let a20 = m[2][0]; let a21 = m[2][1]; let a22 = m[2][2]; let a23 = m[2][3];");
		statement("let a30 = m[3][0]; let a31 = m[3][1]; let a32 = m[3][2]; let a33 = m[3][3];");
		statement("let b00 = a00 * a11 - a01 * a10;");
		statement("let b01 = a00 * a12 - a02 * a10;");
		statement("let b02 = a00 * a13 - a03 * a10;");
		statement("let b03 = a01 * a12 - a02 * a11;");
		statement("let b04 = a01 * a13 - a03 * a11;");
		statement("let b05 = a02 * a13 - a03 * a12;");
		statement("let b06 = a20 * a31 - a21 * a30;");
		statement("let b07 = a20 * a32 - a22 * a30;");
		statement("let b08 = a20 * a33 - a23 * a30;");
		statement("let b09 = a21 * a32 - a22 * a31;");
		statement("let b10 = a21 * a33 - a23 * a31;");
		statement("let b11 = a22 * a33 - a23 * a32;");
		statement("let det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;");
		statement("return mat4x4f(");
		statement("    a11 * b11 - a12 * b10 + a13 * b09, a02 * b10 - a01 * b11 - a03 * b09, "
		          "a31 * b05 - a32 * b04 + a33 * b03, a22 * b04 - a21 * b05 - a23 * b03,");
		statement("    a12 * b08 - a10 * b11 - a13 * b07, a00 * b11 - a02 * b08 + a03 * b07, "
		          "a32 * b02 - a30 * b05 - a33 * b01, a20 * b05 - a22 * b02 + a23 * b01,");
		statement("    a10 * b10 - a11 * b08 + a13 * b06, a01 * b08 - a00 * b10 - a03 * b06, "
		          "a30 * b04 - a31 * b02 + a33 * b00, a21 * b02 - a20 * b04 - a23 * b00,");
		statement("    a11 * b07 - a10 * b09 - a12 * b06, a00 * b09 - a01 * b07 + a02 * b06, "
		          "a31 * b01 - a30 * b03 - a32 * b00, a20 * b03 - a21 * b01 + a22 * b00) * (1.0f / det);");
		end_scope();
		statement("");
	}
}

void CompilerWGSL::replace_illegal_names()
{
	static const unordered_set<string> keywords = {
		// Keywords
		"alias",
		"break",
		"case",
		"const",
		"const_assert",
		"continue",
		"continuing",
		"default",
		"diagnostic",
		"discard",
		"else",
		"enable",
		"false",
		"fn",
		"for",
		"if",
		"let",
		"loop",
		"override",
		"requires",
		"return",
		"struct",
		"switch",
		"true",
		"var",
		"while",
		// Reserved words
		"NULL",
		"Self",
		"abstract",
		"active",
		"alignas",
		"alignof",
		"as",
		"asm",
		"asm_fragment",
		"async",
		"attribute",
		"auto",
		"await",
		"become",
		"cast",
		"catch",
		"class",
		"co_await",
		"co_return",
		"co_yield",
		"coherent",
		"column_major",
		"common",
		"compile",
		"compile_fragment",
		"concept",
		"const_cast",
		"consteval",
		"constexpr",
		"constinit",
		"crate",
		"debugger",
		"decltype",
		"delete",
		"demote",
		"demote_to_helper",
		"do",
		"dynamic_cast",
		"enum",
		"explicit",
		"export",
		"extends",
		"extern",
		"external",
		"fallthrough",
		"filter",
		"final",
		"finally",
		"friend",
		"from",
		"fxgroup",
		"get",
		"goto",
		"groupshared",
		"highp",
		"impl",
		"implements",
		"import",
		"inline",
		"instanceof",
		"interface",
		"layout",
		"lowp",
		"macro",
		"macro_rules",
		"match",
		"mediump",
		"meta",
		"mod",
		"module",
		"move",
		"mut",
		"mutable",
		"namespace",
		"new",
		"nil",
		"noexcept",
		"noinline",
		"nointerpolation",
		"non_coherent",
		"noncoherent",
		"noperspective",
		"null",
		"nullptr",
		"of",
		"operator",
		"package",
		"packoffset",
		"partition",
		"pass",
		"patch",
		"pixelfragment",
		"precise",
		"precision",
		"premerge",
		"priv",
		"protected",
		"pub",
		"public",
		"readonly",
		"ref",
		"regardless",
		"register",
		"reinterpret_cast",
		"require",
		"resource",
		"restrict",
		"self",
		"set",
		"shared",
		"sizeof",
		"smooth",
		"snorm",
		"static",
		"static_assert",
		"static_cast",
		"std",
		"subroutine",
		"super",
		"target",
		"template",
		"this",
		"thread_local",
		"throw",
		"trait",
		"try",
		"type",
		"typedef",
		"typeid",
		"typename",
		"typeof",
		"union",
		"unless",
		"unorm",
		"unsafe",
		"unsized",
		"use",
		"using",
		"varying",
		"virtual",
		"volatile",
		"wgsl",
		"where",
		"with",
		"writeonly",
		"yield",
		// Predeclared types and enumerants. These can be shadowed, but it is confusing and it breaks the generated code.
		"bool",
		"f16",
		"f32",
		"i32",
		"u32",
		"vec2",
		"vec3",
		"vec4",
		"vec2f",
		"vec3f",
		"vec4f",
		"vec2i",
		"vec3i",
		"vec4i",
		"vec2u",
		"vec3u",
		"vec4u",
		"vec2h",
		"vec3h",
		"vec4h",
		"mat2x2",
		"mat2x3",
		"mat2x4",
		"mat3x2",
		"mat3x3",
		"mat3x4",
		"mat4x2",
		"mat4x3",
		"mat4x4",
		"mat2x2f",
		"mat2x3f",
		"mat2x4f",
		"mat3x2f",
		"mat3x3f",
		"mat3x4f",
		"mat4x2f",
		"mat4x3f",
		"mat4x4f",
		"mat2x2h",
		"mat2x3h",
		"mat2x4h",
		"mat3x2h",
		"mat3x3h",
		"mat3x4h",
		"mat4x2h",
		"mat4x3h",
		"mat4x4h",
		"array",
		"atomic",
		"ptr",
		"sampler",
		"sampler_comparison",
		"texture_1d",
		"texture_2d",
		"texture_2d_array",
		"texture_3d",
		"texture_cube",
		"texture_cube_array",
		"texture_multisampled_2d",
		"texture_depth_2d",
		"texture_depth_2d_array",
		"texture_depth_cube",
		"texture_depth_cube_array",
		"texture_depth_multisampled_2d",
		"texture_storage_1d",
		"texture_storage_2d",
		"texture_storage_2d_array",
		"texture_storage_3d",
		"texture_external",
		"function",
		"private",
		"workgroup",
		"uniform",
		"storage",
		"read",
		"write",
		"read_write",
		// Builtin functions.
		"all",
		"any",
		"select",
		"arrayLength",
		"abs",
		"acos",
		"acosh",
		"asin",
		"asinh",
		"atan",
		"atanh",
		"atan2",
		"ceil",
		"clamp",
		"cos",
		"cosh",
		"countLeadingZeros",
		"countOneBits",
		"countTrailingZeros",
		"cross",
		"degrees",
		"determinant",
		"distance",
		"dot",
		"exp",
		"exp2",
		"extractBits",
		"faceForward",
		"firstLeadingBit",
		"firstTrailingBit",
		"floor",
		"fma",
		"fract",
		"frexp",
		"insertBits",
		"inverseSqrt",
		"ldexp",
		"length",
		"log",
		"log2",
		"max",
		"min",
		"mix",
		"modf",
		"normalize",
		"pow",
		"quantizeToF16",
		"radians",
		"reflect",
		"refract",
		"reverseBits",
		"round",
		"saturate",
		"sign",
		"sin",
		"sinh",
		"smoothstep",
		"sqrt",
		"step",
		"tan",
		"tanh",
		"transpose",
		"trunc",
		"dpdx",
		"dpdxCoarse",
		"dpdxFine",
		"dpdy",
		"dpdyCoarse",
		"dpdyFine",
		"fwidth",
		"fwidthCoarse",
		"fwidthFine",
		"textureDimensions",
		"textureGather",
		"textureGatherCompare",
		"textureLoad",
		"textureNumLayers",
		"textureNumLevels",
		"textureNumSamples",
		"textureSample",
		"textureSampleBias",
		"textureSampleCompare",
		"textureSampleCompareLevel",
		"textureSampleGrad",
		"textureSampleLevel",
		"textureSampleBaseClampToEdge",
		"textureStore",
		"atomicLoad",
		"atomicStore",
		"atomicAdd",
		"atomicSub",
		"atomicMax",
		"atomicMin",
		"atomicAnd",
		"atomicOr",
		"atomicXor",
		"atomicExchange",
		"atomicCompareExchangeWeak",
		"pack4x8snorm",
		"pack4x8unorm",
		"pack2x16snorm",
		"pack2x16unorm",
		"pack2x16float",
		"unpack4x8snorm",
		"unpack4x8unorm",
		"unpack2x16snorm",
		"unpack2x16unorm",
		"unpack2x16float",
		"storageBarrier",
		"textureBarrier",
		"workgroupBarrier",
		"workgroupUniformLoad",
		"bitcast",
		// Names used by the backend.
		"stage_input",
		"stage_output",
		"SPIRV_Cross_Input",
		"SPIRV_Cross_Output",
		"vert_main",
		"frag_main",
		"comp_main",
	};

	CompilerGLSL::replace_illegal_names(keywords);
	CompilerGLSL::replace_illegal_names();

	// WGSL does not allow "_" as an identifier, or identifiers starting with "__".
	auto fixup = [](string &name)
	{
		if (name == "_")
			name.clear();
		else if (name.compare(0, 2, "__") == 0)
			name = "u" + name;
	};

	ir.for_each_typed_id<SPIRVariable>([&](uint32_t id, SPIRVariable &) { fixup(ir.meta[id].decoration.alias); });
	ir.for_each_typed_id<SPIRFunction>([&](uint32_t id, SPIRFunction &) { fixup(ir.meta[id].decoration.alias); });
	ir.for_each_typed_id<SPIRType>(
	    [&](uint32_t id, SPIRType &type)
	    {
		    fixup(ir.meta[id].decoration.alias);
		    for (auto &member : ir.meta[type.self].members)
		    {
			    fixup(member.alias);
		    }
	    });
}

void CompilerWGSL::emit_block_hints(const SPIRBlock &)
{
	// WGSL has no control flow attributes.
}

string CompilerWGSL::to_name(uint32_t id, bool allow_alias) const
{
	if (pointer_parameters.count(id))
		return join("(*", CompilerGLSL::to_name(id, allow_alias), ")");
	return CompilerGLSL::to_name(id, allow_alias);
}

string CompilerWGSL::to_qualifiers_glsl(uint32_t)
{
	return "";
}

string CompilerWGSL::scalar_type_name(const SPIRType &type) const
{
	switch (type.basetype)
	{
	case SPIRType::Boolean:
		return "bool";
	case SPIRType::Int:
		return "i32";
	case SPIRType::UInt:
		return "u32";
	case SPIRType::Float:
		return "f32";
	case SPIRType::Half:
		return "f16";
	default:
		SPIRV_CROSS_THROW(join("Scalar type ", base_type_name(type), " is not supported in WGSL."));
	}
}

string CompilerWGSL::base_type_name(const SPIRType &type)
{
	switch (type.basetype)
	{
	case SPIRType::SByte:
		return "int8";
	case SPIRType::UByte:
		return "uint8";
	case SPIRType::Short:
		return "int16";
	case SPIRType::UShort:
		return "uint16";
	case SPIRType::Int64:
		return "int64";
	case SPIRType::UInt64:
		return "uint64";
	case SPIRType::Double:
		return "double";
	case SPIRType::BFloat16:
		return "bfloat16";
	case SPIRType::AtomicCounter:
		return "atomic counter";
	case SPIRType::AccelerationStructure:
		return "acceleration structure";
	case SPIRType::RayQuery:
		return "ray query";
	default:
		break;
	}

	if (type.op == OpTypeCooperativeMatrixKHR)
		return "cooperative matrix";
	return join("(base type ", uint32_t(type.basetype), ")");
}

string CompilerWGSL::type_to_glsl(const SPIRType &type, uint32_t id)
{
	if (is_physical_pointer(type))
		SPIRV_CROSS_THROW("Buffer device addresses are not supported in WGSL.");

	if (type.pointer && type.parent_type)
		return type_to_glsl(get<SPIRType>(type.parent_type), id);

	if (!type.array.empty())
	{
		auto &element = get<SPIRType>(type.parent_type);
		string elem = type_to_glsl(element, id);
		if (!type.array_size_literal.back())
			return join("array<", elem, ", ", to_expression(type.array.back()), ">");
		else if (type.array.back() == 0)
			return join("array<", elem, ">");
		else
			return join("array<", elem, ", ", type.array.back(), ">");
	}

	switch (type.basetype)
	{
	case SPIRType::Struct:
		return to_name(type.self);

	case SPIRType::Image:
	case SPIRType::SampledImage:
		return image_type_glsl(type, id);

	case SPIRType::Sampler:
		return is_comparison_sampler(id) ? "sampler_comparison" : "sampler";

	case SPIRType::Void:
		return "void";

	case SPIRType::Half:
		if (!requires_f16)
		{
			requires_f16 = true;
			force_recompile();
		}
		break;

	case SPIRType::Boolean:
	case SPIRType::Int:
	case SPIRType::UInt:
	case SPIRType::Float:
		break;

	case SPIRType::Short:
	case SPIRType::UShort:
		if (lower_transient_16bit_integers)
		{
			auto wide_type = type;
			wide_type.basetype = type.basetype == SPIRType::Short ? SPIRType::Int : SPIRType::UInt;
			wide_type.width = 32;
			return type_to_glsl(wide_type, id);
		}
		SPIRV_CROSS_THROW(join("Type ", base_type_name(type), " is not supported in WGSL."));

	default:
		SPIRV_CROSS_THROW(join("Type ", base_type_name(type), " is not supported in WGSL."));
	}

	string scalar = scalar_type_name(type);
	const char *suffix = nullptr;
	switch (type.basetype)
	{
	case SPIRType::Int:
		suffix = "i";
		break;
	case SPIRType::UInt:
		suffix = "u";
		break;
	case SPIRType::Float:
		suffix = "f";
		break;
	case SPIRType::Half:
		suffix = "h";
		break;
	default:
		break;
	}

	if (type.vecsize > 4 || type.columns > 4)
		SPIRV_CROSS_THROW("Vectors and matrices with more than 4 components are not supported in WGSL.");

	if (type.columns > 1)
	{
		if (suffix)
			return join("mat", type.columns, "x", type.vecsize, suffix);
		else
			return join("mat", type.columns, "x", type.vecsize, "<", scalar, ">");
	}
	else if (type.vecsize > 1)
	{
		if (suffix)
			return join("vec", type.vecsize, suffix);
		else
			return join("vec", type.vecsize, "<", scalar, ">");
	}
	else
		return scalar;
}

string CompilerWGSL::type_to_array_glsl(const SPIRType &, uint32_t)
{
	// Arrays are part of the type in WGSL.
	return "";
}

bool CompilerWGSL::is_depth_texture(uint32_t id)
{
	if (auto *combined = maybe_get<SPIRCombinedImageSampler>(id))
		id = combined->image;

	auto &type = expression_type(id);
	if (is_depth_image(type, id))
		return true;

	if (auto *var = maybe_get_backing_variable(id))
		return is_depth_image(get<SPIRType>(var->basetype), var->self);

	return false;
}

bool CompilerWGSL::is_comparison_sampler(uint32_t id) const
{
	return id != 0 && comparison_ids.count(id) != 0;
}

string CompilerWGSL::image_format_to_wgsl(ImageFormat fmt) const
{
	switch (fmt)
	{
	case ImageFormatRgba8:
		return "rgba8unorm";
	case ImageFormatRgba8Snorm:
		return "rgba8snorm";
	case ImageFormatRgba8ui:
		return "rgba8uint";
	case ImageFormatRgba8i:
		return "rgba8sint";
	case ImageFormatRgba16ui:
		return "rgba16uint";
	case ImageFormatRgba16i:
		return "rgba16sint";
	case ImageFormatRgba16f:
		return "rgba16float";
	case ImageFormatR32ui:
		return "r32uint";
	case ImageFormatR32i:
		return "r32sint";
	case ImageFormatR32f:
		return "r32float";
	case ImageFormatRg32ui:
		return "rg32uint";
	case ImageFormatRg32i:
		return "rg32sint";
	case ImageFormatRg32f:
		return "rg32float";
	case ImageFormatRgba32ui:
		return "rgba32uint";
	case ImageFormatRgba32i:
		return "rgba32sint";
	case ImageFormatRgba32f:
		return "rgba32float";
	case ImageFormatR8:
		return "r8unorm";
	case ImageFormatR8Snorm:
		return "r8snorm";
	case ImageFormatR8ui:
		return "r8uint";
	case ImageFormatR8i:
		return "r8sint";
	case ImageFormatRg8:
		return "rg8unorm";
	case ImageFormatRg8Snorm:
		return "rg8snorm";
	case ImageFormatRg8ui:
		return "rg8uint";
	case ImageFormatRg8i:
		return "rg8sint";
	case ImageFormatR16ui:
		return "r16uint";
	case ImageFormatR16i:
		return "r16sint";
	case ImageFormatR16f:
		return "r16float";
	case ImageFormatRg16ui:
		return "rg16uint";
	case ImageFormatRg16i:
		return "rg16sint";
	case ImageFormatRg16f:
		return "rg16float";
	case ImageFormatRgb10A2:
		return "rgb10a2unorm";
	case ImageFormatRgb10a2ui:
		return "rgb10a2uint";
	case ImageFormatR11fG11fB10f:
		return "rg11b10ufloat";
	default:
		SPIRV_CROSS_THROW("Storage image format is not supported in WGSL.");
	}
}

string CompilerWGSL::image_access_to_wgsl(const SPIRType &, uint32_t id)
{
	auto *var = maybe_get_backing_variable(id);
	uint32_t var_id = var ? uint32_t(var->self) : id;
	bool non_readable = has_decoration(var_id, DecorationNonReadable);
	bool non_writable = has_decoration(var_id, DecorationNonWritable);
	if (non_readable && !non_writable)
		return "write";
	else if (non_writable && !non_readable)
		return "read";
	else
		return "read_write";
}

string CompilerWGSL::image_type_glsl(const SPIRType &type, uint32_t id, bool)
{
	auto &imagetype = get<SPIRType>(type.image.type);
	string dim;
	switch (type.image.dim)
	{
	case Dim1D:
		dim = "1d";
		break;
	case Dim2D:
		dim = "2d";
		break;
	case Dim3D:
		dim = "3d";
		break;
	case DimCube:
		dim = "cube";
		break;
	default:
		SPIRV_CROSS_THROW("Image dimension is not supported in WGSL.");
	}

	if (type.image.arrayed)
	{
		if (type.image.dim != Dim2D && type.image.dim != DimCube)
			SPIRV_CROSS_THROW("Only 2D and cube textures can be arrayed in WGSL.");
		dim += "_array";
	}

	if (type.image.sampled == 2)
	{
		if (type.image.ms)
			SPIRV_CROSS_THROW("Multisampled storage textures are not supported in WGSL.");
		if (type.image.dim == DimCube)
			SPIRV_CROSS_THROW("Cube storage textures are not supported in WGSL.");
		return join("texture_storage_", dim, "<", image_format_to_wgsl(type.image.format), ", ",
		            image_access_to_wgsl(type, id), ">");
	}

	bool depth = is_depth_image(type, id);
	if (type.image.ms)
	{
		if (type.image.dim != Dim2D || type.image.arrayed)
			SPIRV_CROSS_THROW("Only non-arrayed 2D multisampled textures are supported in WGSL.");
		return depth ? "texture_depth_multisampled_2d" :
		               join("texture_multisampled_2d<", scalar_type_name(imagetype), ">");
	}

	if (depth)
	{
		if (type.image.dim != Dim2D && type.image.dim != DimCube)
			SPIRV_CROSS_THROW("Only 2D and cube depth textures are supported in WGSL.");
		return join("texture_depth_", dim);
	}

	return join("texture_", dim, "<", scalar_type_name(imagetype), ">");
}

string CompilerWGSL::wrap_atomic(const SPIRType &type, const string &base)
{
	if (!type.array.empty())
	{
		string elem = wrap_atomic(get<SPIRType>(type.parent_type), base);
		if (!type.array_size_literal.back())
			return join("array<", elem, ", ", to_expression(type.array.back()), ">");
		else if (type.array.back() == 0)
			return join("array<", elem, ">");
		else
			return join("array<", elem, ", ", type.array.back(), ">");
	}

	if (type.basetype != SPIRType::Int && type.basetype != SPIRType::UInt)
		return base;
	if (type.vecsize != 1)
		SPIRV_CROSS_THROW("Atomics are only supported on scalar types in WGSL.");
	return join("atomic<", scalar_type_name(type), ">");
}

string CompilerWGSL::address_space(StorageClass storage) const
{
	switch (storage)
	{
	case StorageClassFunction:
		return "function";
	case StorageClassPrivate:
	case StorageClassInput:
	case StorageClassOutput:
		return "private";
	case StorageClassWorkgroup:
		return "workgroup";
	case StorageClassUniform:
	case StorageClassPushConstant:
		return "uniform";
	case StorageClassStorageBuffer:
		return "storage";
	default:
		SPIRV_CROSS_THROW("Address space is not supported in WGSL.");
	}
}

string CompilerWGSL::ptr_type(const SPIRType &pointee, StorageClass storage, uint32_t id)
{
	string space = address_space(storage);
	string base = type_to_glsl(pointee, id);
	if (space == "storage")
		return join("ptr<storage, ", base, ", read_write>");
	return join("ptr<", space, ", ", base, ">");
}

string CompilerWGSL::variable_decl(const SPIRType &type, const string &name, uint32_t id)
{
	return join("var ", name, " : ", type_to_glsl(type, id));
}

string CompilerWGSL::variable_decl(const SPIRVariable &variable)
{
	auto &type = get_variable_data_type(variable);
	string space;
	switch (variable.storage)
	{
	case StorageClassPrivate:
	case StorageClassInput:
	case StorageClassOutput:
		space = "<private>";
		break;
	case StorageClassWorkgroup:
		space = "<workgroup>";
		break;
	default:
		break;
	}

	string type_name = variable_is_atomic(variable.self) ? wrap_atomic(type, type_to_glsl(type, variable.self)) :
	                                                       type_to_glsl(type, variable.self);
	string initializer;
	if (variable.loop_variable && variable.static_expression)
	{
		uint32_t expr = variable.static_expression;
		if (ir.ids[expr].get_type() != TypeUndef)
			initializer = to_unpacked_expression(variable.static_expression);
	}
	else if (variable.initializer && variable.storage != StorageClassWorkgroup)
	{
		uint32_t expr = variable.initializer;
		if (ir.ids[expr].get_type() != TypeUndef)
			initializer = to_initializer_expression(variable);
	}

	// Function local variables infer their type from the initializer. Module scope declarations keep it.
	if (!initializer.empty() && variable.storage == StorageClassFunction)
		return join("var ", to_name(variable.self), " = ", initializer);

	auto res = join("var", space, " ", to_name(variable.self), " : ", type_name);
	if (!initializer.empty())
		res += join(" = ", initializer);
	return res;
}

string CompilerWGSL::to_initializer_expression(const SPIRVariable &var)
{
	return to_unpacked_expression(var.initializer);
}

string CompilerWGSL::builtin_to_glsl(BuiltIn builtin, StorageClass storage)
{
	switch (builtin)
	{
	case BuiltInPosition:
		return "gl_Position";
	case BuiltInPointSize:
		return "gl_PointSize";
	case BuiltInClipDistance:
		return "gl_ClipDistance";
	case BuiltInFragCoord:
		return "gl_FragCoord";
	case BuiltInFragDepth:
		return "gl_FragDepth";
	case BuiltInFrontFacing:
		return "gl_FrontFacing";
	case BuiltInVertexIndex:
		return "gl_VertexIndex";
	case BuiltInInstanceIndex:
		return "gl_InstanceIndex";
	case BuiltInSampleId:
		return "gl_SampleID";
	case BuiltInSampleMask:
		return storage == StorageClassInput ? "gl_SampleMaskIn" : "gl_SampleMask";
	case BuiltInLocalInvocationId:
		return "gl_LocalInvocationID";
	case BuiltInLocalInvocationIndex:
		return "gl_LocalInvocationIndex";
	case BuiltInGlobalInvocationId:
		return "gl_GlobalInvocationID";
	case BuiltInWorkgroupId:
		return "gl_WorkGroupID";
	case BuiltInNumWorkgroups:
		return "gl_NumWorkGroups";
	case BuiltInWorkgroupSize:
	{
		SpecializationConstant wg_x, wg_y, wg_z;
		ID workgroup_size_id = get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);
		if (workgroup_size_id)
		{
			auto &c = get<SPIRConstant>(workgroup_size_id);
			if (c.specialization)
				return constant_expression(c);
		}
		require_enable(uses_workgroup_size_constant);
		return "gl_WorkGroupSize";
	}
	case BuiltInSubgroupSize:
		return "gl_SubgroupSize";
	case BuiltInSubgroupLocalInvocationId:
		return "gl_SubgroupInvocationID";
	default:
		return CompilerGLSL::builtin_to_glsl(builtin, storage);
	}
}

string CompilerWGSL::wgsl_builtin_name(BuiltIn builtin, StorageClass storage) const
{
	auto &execution = get_entry_point();
	switch (builtin)
	{
	case BuiltInPosition:
		if (execution.model != ExecutionModelVertex || storage != StorageClassOutput)
			return "";
		return "position";
	case BuiltInFragCoord:
		return "position";
	case BuiltInFragDepth:
		return "frag_depth";
	case BuiltInFrontFacing:
		return "front_facing";
	case BuiltInVertexIndex:
		return "vertex_index";
	case BuiltInInstanceIndex:
		return "instance_index";
	case BuiltInSampleId:
		return "sample_index";
	case BuiltInSampleMask:
		return "sample_mask";
	case BuiltInLocalInvocationId:
		return "local_invocation_id";
	case BuiltInLocalInvocationIndex:
		return "local_invocation_index";
	case BuiltInGlobalInvocationId:
		return "global_invocation_id";
	case BuiltInWorkgroupId:
		return "workgroup_id";
	case BuiltInNumWorkgroups:
		return "num_workgroups";
	case BuiltInSubgroupSize:
		return "subgroup_size";
	case BuiltInSubgroupLocalInvocationId:
		return "subgroup_invocation_id";
	case BuiltInClipDistance:
		if (execution.model != ExecutionModelVertex || storage != StorageClassOutput)
			return "";
		return "clip_distances";
	default:
		return "";
	}
}

string CompilerWGSL::wgsl_builtin_type(BuiltIn builtin) const
{
	switch (builtin)
	{
	case BuiltInPosition:
	case BuiltInFragCoord:
		return "vec4f";
	case BuiltInFragDepth:
		return "f32";
	case BuiltInFrontFacing:
		return "bool";
	case BuiltInLocalInvocationId:
	case BuiltInGlobalInvocationId:
	case BuiltInWorkgroupId:
	case BuiltInNumWorkgroups:
		return "vec3u";
	case BuiltInClipDistance:
		return join("array<f32, ", clip_distance_count, ">");
	default:
		return "u32";
	}
}

string CompilerWGSL::bitcast_glsl_op(const SPIRType &out_type, const SPIRType &in_type)
{
	if (out_type.basetype == in_type.basetype && out_type.width == in_type.width)
		return "";
	return join("bitcast<", type_to_glsl(out_type), ">");
}

string CompilerWGSL::non_finite_float_expression(uint32_t bits)
{
	require_enable(requires_non_finite_helper);

	const char *description = "nan";
	if (bits == 0x7f800000u)
		description = "inf";
	else if (bits == 0xff800000u)
		description = "-inf";

	warn(WGSL_WARNING_NON_FINITE_CONSTANT,
	     join("Constant ", description, " ", warning_location(),
	          " is computed at runtime with spvNonFinite(), since WGSL does not allow infinity or NaN in "
	          "constant expressions.",
	          current_function ? "" : " Module scope constants cannot call functions, so the output will not validate."));

	char print_buffer[32];
	snprintf(print_buffer, sizeof(print_buffer), "0x%xu", bits);
	return join("spvNonFinite(", print_buffer, " /* ", description, " */)");
}

string CompilerWGSL::convert_float_to_string(const SPIRConstant &c, uint32_t col, uint32_t row)
{
	float value = c.scalar_f32(col, row);
	if (get<SPIRType>(c.constant_type).basetype == SPIRType::Float && (std::isnan(value) || std::isinf(value)))
		return non_finite_float_expression(c.scalar(col, row));
	return CompilerGLSL::convert_float_to_string(c, col, row);
}

string CompilerWGSL::convert_half_to_string(const SPIRConstant &c, uint32_t col, uint32_t row)
{
	if (get<SPIRType>(c.constant_type).basetype != SPIRType::Half)
		return CompilerGLSL::convert_half_to_string(c, col, row);

	float value = c.scalar_f16(col, row);
	if (std::isinf(value))
		return join("f16(", non_finite_float_expression(value > 0.0f ? 0x7f800000u : 0xff800000u), ")");
	else if (std::isnan(value))
		return join("f16(", non_finite_float_expression(0x7fc00000u), ")");
	return join(format_float(value), "h");
}

string CompilerWGSL::to_ternary_expression(const SPIRType &, uint32_t select, uint32_t true_value, uint32_t false_value)
{
	return join("select(", to_unpacked_expression(false_value), ", ", to_unpacked_expression(true_value), ", ",
	            to_unpacked_expression(select), ")");
}

bool CompilerWGSL::is_pointer_parameter(const SPIRFunction::Parameter &arg) const
{
	auto &type = expression_type(arg.id);
	if (!type.pointer || arg.write_count == 0)
		return false;

	auto &pointee = get_pointee_type(type);
	if (pointee.basetype == SPIRType::Image || pointee.basetype == SPIRType::SampledImage ||
	    pointee.basetype == SPIRType::Sampler)
		return false;

	switch (type.storage)
	{
	case StorageClassFunction:
	case StorageClassPrivate:
	case StorageClassInput:
	case StorageClassOutput:
	case StorageClassWorkgroup:
	case StorageClassStorageBuffer:
		return true;
	default:
		return false;
	}
}

bool CompilerWGSL::skip_argument(uint32_t) const
{
	return false;
}

void CompilerWGSL::emit_function_prototype(SPIRFunction &func, const Bitset &)
{
	const bool is_entry_point = !ir.is_library_module && func.self == ir.default_entry_point;
	if (!is_entry_point)
		add_function_overload(func);

	// Avoid shadow declarations.
	local_variable_names = resource_names;

	string decl = "fn ";
	if (is_entry_point)
	{
		decl += get_inner_entry_point_name();
		processing_entry_point = true;
	}
	else
		decl += to_name(func.self);

	decl += "(";
	SmallVector<string> arglist;
	for (auto &arg : func.arguments)
	{
		add_local_variable_name(arg.id);

		auto &type = expression_type(arg.id);
		string name = CompilerGLSL::to_name(arg.id);

		if (is_pointer_parameter(arg))
		{
			pointer_parameters.insert(arg.id);
			arglist.push_back(join(name, " : ", ptr_type(get_pointee_type(type), type.storage, arg.id)));
		}
		else if (type.basetype == SPIRType::SampledImage)
		{
			arglist.push_back(join(name, " : ", type_to_glsl(type, arg.id)));
			arglist.push_back(
			    join(name, "_sampler : ", is_comparison_sampler(arg.id) ? "sampler_comparison" : "sampler"));
		}
		else
			arglist.push_back(join(name, " : ", type_to_glsl(type, arg.id)));

		// Hold a pointer to the parameter so we can invalidate the readonly field if needed.
		auto *var = maybe_get<SPIRVariable>(arg.id);
		if (var)
			var->parameter = &arg;
	}

	decl += merge(arglist);
	decl += ")";

	auto &return_type = get<SPIRType>(func.return_type);
	if (return_type.basetype != SPIRType::Void)
		decl += join(" -> ", type_to_glsl(return_type));

	statement(decl);
}

string CompilerWGSL::to_func_call_arg(const SPIRFunction::Parameter &arg, uint32_t id)
{
	if (is_pointer_parameter(arg))
		return address_of_expression(to_expression(id));

	auto &type = expression_type(id);
	if (type.basetype == SPIRType::SampledImage)
		return join(to_image_expression(id), ", ", to_sampler_expression(id));

	// Make sure that we use the name of the original variable, and not the parameter alias.
	uint32_t name_id = id;
	auto *var = maybe_get<SPIRVariable>(id);
	if (var && var->basevariable)
		name_id = var->basevariable;
	return to_unpacked_expression(name_id);
}

string CompilerWGSL::to_image_expression(uint32_t id)
{
	auto *combined = maybe_get<SPIRCombinedImageSampler>(id);
	return to_expression(combined ? uint32_t(combined->image) : id);
}

string CompilerWGSL::to_sampler_expression(uint32_t id)
{
	auto *combined = maybe_get<SPIRCombinedImageSampler>(id);
	if (combined && combined->sampler)
		return to_expression(combined->sampler);

	uint32_t expr_id = combined ? uint32_t(combined->image) : id;
	auto img_expr = to_expression(expr_id);
	auto index = img_expr.find_first_of('[');
	if (index == string::npos)
		return img_expr + "_sampler";
	else
		return img_expr.substr(0, index) + "_sampler" + img_expr.substr(index);
}

void CompilerWGSL::emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id)
{
	set<SPIRCombinedImageSampler>(result_id, result_type, image_id, samp_id);
}

// Atomics analysis. WGSL requires atomic types for any memory accessed with atomic operations.

static inline uint64_t atomic_member_key(uint32_t type_id, uint32_t index)
{
	return (uint64_t(type_id) << 32) | index;
}

void CompilerWGSL::analyze_mutable_temporaries()
{
	// SPIR-V values are immutable, so temporaries can be declared with let, except when the GLSL backend
	// implements a composite insert by copying the composite to a temporary and modifying it, or by
	// modifying the inserted-into composite in place.
	mutable_temporaries.clear();
	for (auto &id : ir.ids)
	{
		if (id.get_type() != TypeFunction)
			continue;
		auto &func = id.get<SPIRFunction>();
		for (auto block_id : func.blocks)
		{
			for (auto &i : get<SPIRBlock>(block_id).ops)
			{
				auto *ops = stream(i);
				auto op = static_cast<Op>(i.op);
				if (op == OpCompositeInsert)
				{
					mutable_temporaries.insert(ops[1]);
					mutable_temporaries.insert(ops[3]);
				}
				else if (op == OpVectorInsertDynamic)
				{
					mutable_temporaries.insert(ops[1]);
					mutable_temporaries.insert(ops[2]);
				}
			}
		}
	}
}

void CompilerWGSL::analyze_single_store_variables()
{
	// A function local variable can be declared with let at its first store if that store is the only write,
	// and the variable never escapes, i.e. it is only read with OpLoad, either directly or through access chains.
	single_store_variables.clear();
	variable_call_arguments.clear();
	for (auto &id : ir.ids)
	{
		if (id.get_type() != TypeFunction)
			continue;
		auto &func = id.get<SPIRFunction>();

		unordered_map<uint32_t, uint32_t> store_counts;
		unordered_set<uint32_t> escaped;
		for (auto var_id : func.local_variables)
			store_counts[var_id] = 0;

		// Pointers derived from a local variable through access chains.
		unordered_map<uint32_t, uint32_t> derived;
		auto base_variable = [&](uint32_t ptr) -> uint32_t {
			if (store_counts.count(ptr))
				return ptr;
			auto itr = derived.find(ptr);
			return itr != derived.end() ? itr->second : 0;
		};

		for (auto block_id : func.blocks)
		{
			for (auto &i : get<SPIRBlock>(block_id).ops)
			{
				auto *ops = stream(i);
				auto op = static_cast<Op>(i.op);
				switch (op)
				{
				case OpLoad:
					break;

				case OpAccessChain:
				case OpInBoundsAccessChain:
					if (uint32_t base = base_variable(ops[2]))
						derived[ops[1]] = base;
					// Indices may reference variables too, e.g. by mistake, so check them conservatively.
					for (uint32_t j = 3; j < i.length; j++)
						if (uint32_t base = base_variable(ops[j]))
							escaped.insert(base);
					break;

				case OpFunctionCall:
					for (uint32_t j = 3; j < i.length; j++)
					{
						// Only whole variables can be passed by value, see to_func_call_arg().
						if (store_counts.count(ops[j]))
							variable_call_arguments[ops[j]].push_back({ ops[2], j - 3 });
						else if (uint32_t base = base_variable(ops[j]))
							escaped.insert(base);
					}
					break;

				case OpStore:
					if (store_counts.count(ops[0]))
						store_counts[ops[0]]++;
					else if (uint32_t base = base_variable(ops[0]))
						escaped.insert(base); // Partial writes through an access chain.
					if (uint32_t base = base_variable(ops[1]))
						escaped.insert(base);
					break;

				default:
					// Any other use (function calls, atomics, OpCopyMemory, extended instructions with
					// pointer arguments, ...) may write to the variable. Literals which happen to alias
					// a variable ID only make this conservative.
					for (uint32_t j = 0; j < i.length; j++)
						if (uint32_t base = base_variable(ops[j]))
							escaped.insert(base);
					break;
				}
			}
		}

		for (auto &count : store_counts)
		{
			auto &var = get<SPIRVariable>(count.first);
			if (count.second == 1 && !escaped.count(count.first) && !var.phi_variable && !var.loop_variable &&
			    !var.initializer)
				single_store_variables.insert(count.first);
		}
	}
}

bool CompilerWGSL::variable_can_be_let(uint32_t var_id) const
{
	if (!single_store_variables.count(var_id))
		return false;

	auto itr = variable_call_arguments.find(var_id);
	if (itr == variable_call_arguments.end())
		return true;

	// Callees are emitted before their callers, so parameter write counts are known here.
	for (auto &call : itr->second)
	{
		auto &callee = get<SPIRFunction>(call.first);
		if (call.second >= callee.arguments.size() || is_pointer_parameter(callee.arguments[call.second]))
			return false;
	}
	return true;
}

string CompilerWGSL::declare_temporary(uint32_t result_type, uint32_t result_id)
{
	// Temporaries declared in continue blocks are hoisted to the loop header, and hoisted temporaries
	// are assigned rather than declared. The base class handles both.
	bool hoisted = (!block_temporary_hoisting && current_continue_block && !hoisted_temporaries.count(result_id)) ||
	               hoisted_temporaries.count(result_id);
	if (hoisted)
		return CompilerGLSL::declare_temporary(result_type, result_id);

	// The type is inferred from the initializer, but make sure it is representable in WGSL.
	type_to_glsl(get<SPIRType>(result_type));
	add_local_variable_name(result_id);
	return join(mutable_temporaries.count(result_id) ? "var " : "let ", to_name(result_id), " = ");
}

bool CompilerWGSL::is_16bit_integer_type(uint32_t type_id) const
{
	if (ir.ids[type_id].get_type() != TypeType)
		return false;
	auto *type = &get<SPIRType>(type_id);
	while (type->pointer && type->parent_type)
		type = &get<SPIRType>(type->parent_type);
	return type->basetype == SPIRType::Short || type->basetype == SPIRType::UShort;
}

void CompilerWGSL::analyze_transient_16bit_integers()
{
	// dxc lowers e.g. half(uint_value) to OpUConvert to a 16-bit integer followed by OpConvertUToF.
	// If every 16-bit integer value is produced by a conversion from a 32-bit integer and only consumed
	// by int to float conversions, the 16-bit values can be represented exactly with 32-bit integers.
	lower_transient_16bit_integers = false;
	unordered_set<uint32_t> values;
	bool found = false;

	for (auto &id : ir.ids)
	{
		if (id.get_type() != TypeFunction)
			continue;
		auto &func = id.get<SPIRFunction>();
		for (auto &arg : func.arguments)
			if (is_16bit_integer_type(arg.type))
				return;
		if (is_16bit_integer_type(func.return_type))
			return;

		for (auto block_id : func.blocks)
		{
			for (auto &i : get<SPIRBlock>(block_id).ops)
			{
				auto *ops = stream(i);
				auto op = static_cast<Op>(i.op);
				if (i.length < 2 || !is_16bit_integer_type(ops[0]))
					continue;

				found = true;
				if (op != OpUConvert && op != OpSConvert)
					return;
				values.insert(ops[1]);
			}
		}
	}

	if (!found)
		return;

	for (auto &id : ir.ids)
	{
		if (id.get_type() != TypeFunction)
			continue;
		auto &func = id.get<SPIRFunction>();
		for (auto block_id : func.blocks)
		{
			auto &block = get<SPIRBlock>(block_id);
			for (auto &i : block.ops)
			{
				auto *ops = stream(i);
				auto op = static_cast<Op>(i.op);
				bool allowed_use = op == OpConvertUToF || op == OpConvertSToF;
				bool produces_16bit = i.length >= 2 && is_16bit_integer_type(ops[0]);
				for (uint32_t j = 0; j < i.length; j++)
				{
					// Skip the result ID of the conversions which produce the 16-bit values.
					if (j == 1 && produces_16bit)
						continue;
					if (values.count(ops[j]) && !(allowed_use && j == 2))
						return;
				}
			}

			// Values used as branch conditions, return values, etc.
			if (values.count(block.condition) || values.count(block.return_value))
				return;
		}
	}

	lower_transient_16bit_integers = true;
}

void CompilerWGSL::analyze_atomics()
{
	atomic_members.clear();
	atomic_variables.clear();

	for (auto &id : ir.ids)
	{
		if (id.get_type() != TypeFunction)
			continue;
		auto &func = id.get<SPIRFunction>();
		for (auto block_id : func.blocks)
		{
			auto &block = get<SPIRBlock>(block_id);
			for (auto &i : block.ops)
			{
				auto *ops = stream(i);
				switch (static_cast<Op>(i.op))
				{
				case OpAtomicStore:
					mark_atomic_pointer(ops[0]);
					break;

				case OpAtomicLoad:
				case OpAtomicExchange:
				case OpAtomicCompareExchange:
				case OpAtomicCompareExchangeWeak:
				case OpAtomicIIncrement:
				case OpAtomicIDecrement:
				case OpAtomicIAdd:
				case OpAtomicISub:
				case OpAtomicSMin:
				case OpAtomicUMin:
				case OpAtomicSMax:
				case OpAtomicUMax:
				case OpAtomicAnd:
				case OpAtomicOr:
				case OpAtomicXor:
					mark_atomic_pointer(ops[2]);
					break;

				default:
					break;
				}
			}
		}
	}
}

// Walks an access chain and returns the innermost struct member which contains the accessed element,
// or the base variable if the chain does not go through a struct.
bool CompilerWGSL::resolve_atomic_target(uint32_t ptr_id, uint32_t &type_id, uint32_t &member, uint32_t &var_id)
{
	// Collect the chain of indices back to the base variable.
	SmallVector<uint32_t> indices;
	uint32_t base = ptr_id;
	for (;;)
	{
		if (ir.ids[base].get_type() == TypeVariable)
			break;

		// Find the instruction that defines the access chain.
		bool found = false;
		for (auto &id : ir.ids)
		{
			if (id.get_type() != TypeFunction)
				continue;
			auto &func = id.get<SPIRFunction>();
			for (auto block_id : func.blocks)
			{
				auto &block = get<SPIRBlock>(block_id);
				for (auto &i : block.ops)
				{
					auto op = static_cast<Op>(i.op);
					if ((op == OpAccessChain || op == OpInBoundsAccessChain) && ir.spirv[i.offset + 1] == base)
					{
						const uint32_t *ops = &ir.spirv[i.offset];
						SmallVector<uint32_t> new_indices;
						for (uint32_t j = 3; j < i.length; j++)
							new_indices.push_back(ops[j]);
						new_indices.insert(new_indices.end(), indices.begin(), indices.end());
						indices = std::move(new_indices);
						base = ops[2];
						found = true;
						break;
					}
				}
				if (found)
					break;
			}
			if (found)
				break;
		}

		if (!found)
			return false;
	}

	auto &var = get<SPIRVariable>(base);
	auto *type = &get_variable_data_type(var);

	type_id = 0;
	member = 0;
	var_id = var.self;

	for (auto index : indices)
	{
		if (!type->array.empty())
		{
			type = &get<SPIRType>(type->parent_type);
		}
		else if (type->basetype == SPIRType::Struct)
		{
			member = evaluate_constant_u32(index);
			type_id = type->self;
			var_id = 0;
			type = &get<SPIRType>(type->member_types[member]);
		}
		else
			break;
	}

	return true;
}

void CompilerWGSL::mark_atomic_pointer(uint32_t ptr_id)
{
	uint32_t type_id, member, var_id;
	if (!resolve_atomic_target(ptr_id, type_id, member, var_id))
		return;

	if (var_id)
		atomic_variables.insert(var_id);
	else
		atomic_members.insert(atomic_member_key(type_id, member));
}

bool CompilerWGSL::is_atomic_pointer(uint32_t ptr_id)
{
	if (atomic_members.empty() && atomic_variables.empty())
		return false;

	uint32_t type_id, member, var_id;
	if (!resolve_atomic_target(ptr_id, type_id, member, var_id))
		return false;

	if (var_id)
		return atomic_variables.count(var_id) != 0;
	else
		return member_is_atomic(type_id, member);
}

bool CompilerWGSL::member_is_atomic(uint32_t type_id, uint32_t index) const
{
	return atomic_members.count(atomic_member_key(type_id, index)) != 0;
}

bool CompilerWGSL::variable_is_atomic(uint32_t var_id) const
{
	return atomic_variables.count(var_id) != 0;
}

// Resources

bool CompilerWGSL::member_is_non_native_row_major_matrix(const SPIRType &type, uint32_t index, bool)
{
	// Row-major matrices are declared transposed, so non-square matrices are fine.
	return has_member_decoration(type.self, index, DecorationRowMajor);
}

static bool is_matrix_or_matrix_array(const Compiler &compiler, uint32_t type_id)
{
	auto *type = &compiler.get_type(type_id);
	while (!type->array.empty())
		type = &compiler.get_type(type->parent_type);
	return type->columns > 1;
}

string CompilerWGSL::declared_type_to_wgsl(uint32_t type_id, bool transpose)
{
	auto &type = get<SPIRType>(type_id);
	if (!type.array.empty())
	{
		string elem = declared_type_to_wgsl(type.parent_type, transpose);
		if (!type.array_size_literal.back())
			return join("array<", elem, ", ", to_expression(type.array.back()), ">");
		else if (type.array.back() == 0)
			return join("array<", elem, ">");
		else
			return join("array<", elem, ", ", type.array.back(), ">");
	}

	if (transpose && type.columns > 1)
	{
		auto transposed = type;
		swap(transposed.columns, transposed.vecsize);
		return type_to_glsl(transposed);
	}

	return type_to_glsl(type);
}

void CompilerWGSL::wgsl_layout(uint32_t type_id, bool transpose, uint32_t &align, uint32_t &size)
{
	auto &t = get<SPIRType>(type_id);

	if (!t.array.empty())
	{
		uint32_t ealign, esize;
		wgsl_layout(t.parent_type, transpose, ealign, esize);
		uint32_t stride = round_up(esize, ealign);
		if (has_decoration(type_id, DecorationArrayStride))
		{
			uint32_t decl_stride = get_decoration(type_id, DecorationArrayStride);
			if (decl_stride != stride)
			{
				SPIRV_CROSS_THROW(join("Array stride ", decl_stride,
				                       " cannot be represented in WGSL (natural stride is ", stride, ")."));
			}
		}
		align = ealign;
		uint32_t count = t.array_size_literal.back() ? t.array.back() : 1;
		size = stride * max(count, 1u);
		return;
	}

	if (t.basetype == SPIRType::Struct)
	{
		align = 1;
		size = 0;
		for (uint32_t i = 0; i < uint32_t(t.member_types.size()); i++)
		{
			uint32_t malign, msize;
			uint32_t physical = get_extended_member_decoration(t.self, i, SPIRVCrossDecorationPhysicalTypeID);
			if (physical)
				wgsl_layout(physical, false, malign, msize);
			else
				wgsl_layout(t.member_types[i], has_member_decoration(t.self, i, DecorationRowMajor), malign, msize);
			align = max(align, malign);
			uint32_t offset = has_member_decoration(t.self, i, DecorationOffset) ?
			                      get_member_decoration(t.self, i, DecorationOffset) :
			                      round_up(size, malign);
			size = max(size, offset + msize);
		}
		size = max(round_up(size, align), struct_padded_size(type_id));
		return;
	}

	uint32_t scalar = t.basetype == SPIRType::Boolean ? 4 : t.width / 8;
	uint32_t columns = t.columns;
	uint32_t vecsize = t.vecsize;
	if (transpose && columns > 1)
		swap(columns, vecsize);
	uint32_t vec_align = vecsize == 3 ? 4 * scalar : vecsize * scalar;
	uint32_t vec_size = vecsize * scalar;
	align = vec_align;
	size = columns > 1 ? columns * round_up(vec_size, vec_align) : vec_size;
}

uint32_t CompilerWGSL::struct_padded_size(uint32_t struct_type_id)
{
	// If a struct is used in an array with a larger stride than its size, its last member is padded.
	uint32_t padded_size = 0;
	ir.for_each_typed_id<SPIRType>(
	    [&](uint32_t id, const SPIRType &t)
	    {
		    if (!t.array.empty() && !t.pointer && t.parent_type == struct_type_id &&
		        has_decoration(id, DecorationArrayStride))
		    {
			    padded_size = max(padded_size, get_decoration(id, DecorationArrayStride));
		    }
	    });
	return padded_size;
}

uint32_t CompilerWGSL::build_padded_physical_type(uint32_t type_id, bool transpose)
{
	// Builds a type with the same shape, but with vectors widened so that the WGSL layout
	// matches a 16 byte array or matrix stride (std140).
	// Row-major matrices are declared transposed, so the transposed columns are padded.
	auto &type = get<SPIRType>(type_id);
	uint32_t new_id = ir.increase_bound_by(1);
	if (!type.array.empty())
	{
		uint32_t elem = build_padded_physical_type(type.parent_type, transpose);
		auto &new_type = set<SPIRType>(new_id, get<SPIRType>(type_id));
		auto &elem_type = get<SPIRType>(elem);
		new_type.parent_type = elem;
		new_type.self = elem_type.self;
		new_type.vecsize = elem_type.vecsize;
		new_type.columns = elem_type.columns;
	}
	else
	{
		auto &new_type = set<SPIRType>(new_id, get<SPIRType>(type_id));
		if (transpose && new_type.columns > 1)
			new_type.columns = new_type.vecsize;
		new_type.vecsize = 4;
		new_type.parent_type = 0;
	}
	return new_id;
}

void CompilerWGSL::prepare_buffer_layouts()
{
	SmallVector<uint32_t> struct_types;
	ir.for_each_typed_id<SPIRType>(
	    [&](uint32_t id, SPIRType &type)
	    {
		    if (type.basetype == SPIRType::Struct && type.array.empty() && !type.pointer &&
		        !type.member_types.empty() && has_member_decoration(type.self, 0, DecorationOffset) && id == type.self)
			    struct_types.push_back(id);
	    });

	for (auto struct_id : struct_types)
	{
		for (uint32_t i = 0; i < uint32_t(get<SPIRType>(struct_id).member_types.size()); i++)
		{
			if (has_extended_member_decoration(struct_id, i, SPIRVCrossDecorationPhysicalTypeID))
				continue;

			uint32_t member_type_id = get<SPIRType>(struct_id).member_types[i];
			bool row_major = has_member_decoration(struct_id, i, DecorationRowMajor);

			// Find the innermost element type and check whether any stride needs padding.
			bool needs_padding = false;
			uint32_t elem_id = member_type_id;
			while (!get<SPIRType>(elem_id).array.empty())
			{
				auto &arr = get<SPIRType>(elem_id);
				auto &elem = get<SPIRType>(arr.parent_type);
				if (has_decoration(elem_id, DecorationArrayStride) && elem.basetype != SPIRType::Struct &&
				    elem.array.empty() && elem.columns == 1)
				{
					uint32_t ealign, esize;
					wgsl_layout(arr.parent_type, false, ealign, esize);
					uint32_t stride = get_decoration(elem_id, DecorationArrayStride);
					if (stride != round_up(esize, ealign))
					{
						if (stride != 16 || elem.vecsize == 4 || elem.width != 32)
							SPIRV_CROSS_THROW(join("Array stride ", stride, " cannot be represented in WGSL."));
						needs_padding = true;
					}
				}
				elem_id = arr.parent_type;
			}

			auto *mtype = &get<SPIRType>(elem_id);
			if (mtype->columns > 1 && has_member_decoration(struct_id, i, DecorationMatrixStride))
			{
				uint32_t stride = get_member_decoration(struct_id, i, DecorationMatrixStride);
				uint32_t vecsize = row_major ? mtype->columns : mtype->vecsize;
				uint32_t scalar = mtype->width / 8;
				uint32_t natural = vecsize == 2 ? 2 * scalar : 4 * scalar;
				if (stride != natural)
				{
					if (stride != 16 || vecsize != 2 || scalar != 4)
						SPIRV_CROSS_THROW(join("Matrix stride ", stride, " cannot be represented in WGSL."));
					needs_padding = true;
				}
			}

			if (needs_padding)
			{
				uint32_t physical = build_padded_physical_type(member_type_id, row_major);
				set_extended_member_decoration(struct_id, i, SPIRVCrossDecorationPhysicalTypeID, physical);
			}
		}
	}
}

string CompilerWGSL::convert_row_major_matrix(string exp_str, const SPIRType &exp_type, uint32_t physical_type_id,
                                              bool is_packed, bool relaxed)
{
	if (physical_type_id && exp_type.columns > 1)
	{
		strip_enclosed_expression(exp_str);
		return join("transpose(", unpack_expression_type(exp_str, exp_type, physical_type_id, false, true), ")");
	}
	return CompilerGLSL::convert_row_major_matrix(exp_str, exp_type, physical_type_id, is_packed, relaxed);
}

void CompilerWGSL::emit_store_statement(uint32_t lhs_expression, uint32_t rhs_expression)
{
	// A function local variable declared by its first store infers its type from the stored value.
	auto *var = maybe_get<SPIRVariable>(lhs_expression);
	if (var && var->deferred_declaration && var->storage == StorageClassFunction && !variable_is_atomic(var->self))
	{
		auto rhs = to_pointer_expression(rhs_expression);
		if (!rhs.empty())
		{
			// The type is inferred, but make sure it is representable in WGSL.
			type_to_glsl(get_variable_data_type(*var), var->self);
			var->deferred_declaration = false;
			statement(variable_can_be_let(var->self) ? "let " : "var ", to_name(var->self), " = ", rhs, ";");
			register_write(lhs_expression);
			return;
		}
	}

	auto *e = maybe_get<SPIRExpression>(lhs_expression);
	bool need_transpose = e && e->need_transpose;
	uint32_t physical = get_extended_decoration(lhs_expression, SPIRVCrossDecorationPhysicalTypeID);

	if (!need_transpose && !physical)
	{
		CompilerGLSL::emit_store_statement(lhs_expression, rhs_expression);
		return;
	}

	auto &value_type = expression_type(rhs_expression);
	if (e)
		e->need_transpose = false;
	string lhs = to_dereferenced_expression(lhs_expression);
	if (e)
		e->need_transpose = need_transpose;

	string rhs = to_unpacked_expression(rhs_expression);
	if (!should_forward(rhs_expression) || needs_enclose_expression(rhs))
	{
		// The value is referenced multiple times below.
		auto tmp = join("_", lhs_expression, "_", rhs_expression, "_store");
		statement("let ", tmp, " = ", rhs, ";");
		rhs = tmp;
	}

	if (need_transpose && value_type.columns > 1)
	{
		if (physical)
		{
			// Padded row-major matrix, store the transposed rows element by element.
			for (uint32_t c = 0; c < value_type.columns; c++)
				for (uint32_t r = 0; r < value_type.vecsize; r++)
					statement(lhs, "[", r, "][", c, "] = ", rhs, "[", c, "][", r, "];");
		}
		else
			statement(lhs, " = transpose(", rhs, ");");
	}
	else if (need_transpose)
	{
		// Storing a column of a row-major matrix. Scatter the elements to the rows.
		auto index = lhs.find_last_of('[');
		if (index == string::npos || value_type.vecsize == 1)
			SPIRV_CROSS_THROW("Unsupported store to row-major matrix.");
		auto base = lhs.substr(0, index);
		auto column = lhs.substr(index);
		for (uint32_t r = 0; r < value_type.vecsize; r++)
			statement(base, "[", r, "]", column, " = ", rhs, ".", swizzle_components[r], ";");
	}
	else if (value_type.columns > 1)
	{
		// Padded column-major matrix.
		for (uint32_t c = 0; c < value_type.columns; c++)
			for (uint32_t r = 0; r < value_type.vecsize; r++)
				statement(lhs, "[", c, "][", r, "] = ", rhs, "[", c, "][", r, "];");
	}
	else if (value_type.array.empty())
	{
		// Scalar or vector stored into a padded vector.
		if (value_type.vecsize == 1)
			statement(lhs, ".x = ", rhs, ";");
		else
			for (uint32_t i = 0; i < value_type.vecsize; i++)
				statement(lhs, ".", swizzle_components[i], " = ", rhs, ".", swizzle_components[i], ";");
	}
	else
		SPIRV_CROSS_THROW("Storing arrays to padded buffer members is not supported in WGSL.");

	register_write(lhs_expression);
}

string CompilerWGSL::unpack_expression_type(string expr_str, const SPIRType &type, uint32_t physical_type_id, bool,
                                            bool row_major)
{
	if (physical_type_id == 0)
		return expr_str;

	static const char *swizzle_lut[] = { ".x", ".xy", ".xyz", "" };
	auto &physical_type = get<SPIRType>(physical_type_id);

	if (!type.array.empty())
	{
		// Whole array load, unpack element by element.
		if (!type.array_size_literal.back() || type.array.back() == 0)
			SPIRV_CROSS_THROW("Cannot unpack runtime or specialization constant sized arrays.");
		if (physical_type.array.empty())
			return expr_str;

		auto &elem_type = get<SPIRType>(type.parent_type);
		string unpacked = join(type_to_glsl(type), "(");
		for (uint32_t i = 0; i < type.array.back(); i++)
		{
			if (i)
				unpacked += ", ";
			unpacked += unpack_expression_type(join(enclose_expression(expr_str), "[", i, "]"), elem_type,
			                                   physical_type.parent_type, false, false);
		}
		unpacked += ")";
		return unpacked;
	}

	// Indexing into an array of padded elements.
	const SPIRType *element_physical_type = &physical_type;
	while (!element_physical_type->array.empty())
		element_physical_type = &get<SPIRType>(element_physical_type->parent_type);

	if (type.columns > 1)
	{
		// Padded matrix. Rebuild the matrix from the padded columns.
		// For row-major matrices, this rebuilds the matrix as declared, i.e. transposed.
		auto stored_type = type;
		if (row_major)
			swap(stored_type.columns, stored_type.vecsize);
		if (element_physical_type->vecsize == stored_type.vecsize)
			return expr_str;
		string unpacked = join(type_to_glsl(stored_type), "(");
		for (uint32_t i = 0; i < stored_type.columns; i++)
		{
			if (i)
				unpacked += ", ";
			unpacked += join(enclose_expression(expr_str), "[", i, "]", swizzle_lut[stored_type.vecsize - 1]);
		}
		unpacked += ")";
		return unpacked;
	}

	if ((physical_type.columns > 1 || !physical_type.array.empty() || physical_type.vecsize > type.vecsize) &&
	    type.vecsize < 4)
	{
		// Vector or scalar loaded from a padded vector.
		auto *phys = &physical_type;
		while (!phys->array.empty())
			phys = &get<SPIRType>(phys->parent_type);
		if (phys->vecsize > type.vecsize)
			return enclose_expression(expr_str) + swizzle_lut[type.vecsize - 1];
	}

	return expr_str;
}

void CompilerWGSL::emit_struct_wgsl(SPIRType &type)
{
	// Struct types can be stamped out multiple times with just different offsets, matrix layouts, etc ...
	if (type.type_alias != TypeID(0) &&
	    !has_extended_decoration(type.type_alias, SPIRVCrossDecorationBufferBlockRepacked))
		return;

	add_resource_name(type.self);
	statement("struct ", to_name(type.self));
	begin_scope();

	type.member_name_cache.clear();

	uint32_t member_count = uint32_t(type.member_types.size());
	bool has_offsets = member_count > 0 && has_member_decoration(type.self, 0, DecorationOffset);

	// Compute @size attributes required to match the SPIR-V member offsets.
	SmallVector<uint32_t> size_attrs(member_count);
	if (has_offsets)
	{
		uint32_t offset = 0;
		for (uint32_t i = 0; i < member_count; i++)
		{
			uint32_t malign, msize;
			uint32_t physical = get_extended_member_decoration(type.self, i, SPIRVCrossDecorationPhysicalTypeID);
			if (physical)
				wgsl_layout(physical, false, malign, msize);
			else
				wgsl_layout(type.member_types[i], has_member_decoration(type.self, i, DecorationRowMajor), malign,
				            msize);

			uint32_t decl_offset = get_member_decoration(type.self, i, DecorationOffset);
			uint32_t natural_offset = round_up(offset, malign);

			if (decl_offset != natural_offset)
			{
				if (decl_offset < natural_offset || i == 0 || (decl_offset % malign) != 0)
				{
					SPIRV_CROSS_THROW(join("Member offset ", decl_offset, " of struct ", to_name(type.self),
					                       " cannot be represented in WGSL."));
				}

				uint32_t prev_offset = get_member_decoration(type.self, i - 1, DecorationOffset);
				size_attrs[i - 1] = decl_offset - prev_offset;
			}

			offset = decl_offset + msize;
		}

		// If the struct is used in an array with a larger stride, pad the last member.
		uint32_t struct_size = offset;
		uint32_t padded_size = struct_padded_size(type.self);
		if (padded_size > struct_size)
		{
			uint32_t last_offset = get_member_decoration(type.self, member_count - 1, DecorationOffset);
			size_attrs[member_count - 1] = padded_size - last_offset;
		}
	}

	for (uint32_t i = 0; i < member_count; i++)
	{
		add_member_name(type, i);
		auto &membertype = get<SPIRType>(type.member_types[i]);

		string attrs;
		if (size_attrs[i])
			attrs = join("@size(", size_attrs[i], ") ");

		string type_name;
		uint32_t physical = get_extended_member_decoration(type.self, i, SPIRVCrossDecorationPhysicalTypeID);
		if (physical)
			type_name = declared_type_to_wgsl(physical, false);
		else
			type_name =
			    declared_type_to_wgsl(type.member_types[i], has_member_decoration(type.self, i, DecorationRowMajor) &&
			                                                    is_matrix_or_matrix_array(*this, type.member_types[i]));

		if (member_is_atomic(type.self, i))
			type_name = wrap_atomic(membertype, type_name);

		statement(attrs, to_member_name(type, i), " : ", type_name, ",");
	}

	if (member_count == 0)
		statement("empty_struct_member : i32,");

	end_scope();
	statement("");
}

void CompilerWGSL::emit_constants_and_structs()
{
	SpecializationConstant wg_x, wg_y, wg_z;
	ID workgroup_size_id = get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);

	bool emitted = false;
	auto loop_lock = ir.create_loop_hard_lock();
	for (auto &id_ : ir.ids_for_constant_undef_or_type)
	{
		auto &id = ir.ids[id_];

		if (id.get_type() == TypeConstant)
		{
			auto &c = id.get<SPIRConstant>();
			auto &type = get<SPIRType>(c.constant_type);

			if (c.self == workgroup_size_id)
			{
				// If the workgroup size is specialized, it is an override-expression and is emitted inline.
				// Otherwise only declare it if the shader actually references it.
				if (!c.specialization && uses_workgroup_size_constant)
				{
					statement("const gl_WorkGroupSize : vec3u = ", constant_expression(c), ";");
					emitted = true;
				}
			}
			else if (c.specialization)
			{
				add_resource_name(c.self);
				auto name = to_name(c.self);

				if (type.vecsize == 1 && type.columns == 1 && type.array.empty() && type.basetype != SPIRType::Struct)
				{
					if (has_decoration(c.self, DecorationSpecId))
						statement("@id(", get_decoration(c.self, DecorationSpecId), ") override ", name, " : ",
						          type_to_glsl(type), " = ", constant_expression(c), ";");
					else
						statement("override ", name, " : ", type_to_glsl(type), " = ", constant_expression(c), ";");
				}
				else
				{
					// Only scalars can be overrides. Composites are emitted inline as override-expressions.
					forwarded_temporaries.insert(c.self);
				}
				emitted = true;
			}
			else if (c.is_used_as_lut)
			{
				add_resource_name(c.self);
				statement("const ", to_name(c.self), " : ", type_to_glsl(type), " = ", constant_expression(c), ";");
				emitted = true;
			}
		}
		else if (id.get_type() == TypeConstantOp)
		{
			auto &c = id.get<SPIRConstantOp>();
			auto &type = get<SPIRType>(c.basetype);
			add_resource_name(c.self);
			auto name = to_name(c.self);
			if (type.vecsize == 1 && type.columns == 1 && type.array.empty() && type.basetype != SPIRType::Struct)
			{
				statement("override ", name, " : ", type_to_glsl(type), " = ", constant_op_expression(c), ";");
				emitted = true;
			}
			else
			{
				// Only scalars can be overrides. Composites are emitted inline as override-expressions.
				set_name(c.self, join("(", constant_op_expression(c), ")"));
			}
		}
		else if (id.get_type() == TypeType)
		{
			auto &type = id.get<SPIRType>();
			if (type.basetype == SPIRType::Struct && type.array.empty() && !type.pointer && !is_builtin_type(type))
			{
				if (emitted)
					statement("");
				emitted = false;
				emit_struct_wgsl(type);
			}
		}
		else if (id.get_type() == TypeUndef)
		{
			auto &undef = id.get<SPIRUndef>();
			auto &type = get<SPIRType>(undef.basetype);
			if (type.basetype == SPIRType::Void || type_is_top_level_block(type) || type.pointer ||
			    type_is_opaque_value(type))
				continue;
			statement("var<private> ", to_name(undef.self), " : ", type_to_glsl(type), ";");
			emitted = true;
		}
	}

	if (emitted)
		statement("");
}

string CompilerWGSL::binding_attributes(uint32_t var_id, bool sampler_part)
{
	uint32_t group = get_decoration(var_id, DecorationDescriptorSet);
	uint32_t binding = get_decoration(var_id, DecorationBinding);

	if (wgsl_options.resolve_binding_conflicts)
	{
		auto &bindings = sampler_part ? resolved_sampler_bindings : resolved_bindings;
		auto itr = bindings.find(var_id);
		if (itr != bindings.end())
			binding = itr->second;
	}
	else if (sampler_part)
		binding += wgsl_options.combined_sampler_binding_offset;

	return join("@group(", group, ") @binding(", binding, ") ");
}

void CompilerWGSL::resolve_binding_conflicts()
{
	// Mirrors tint's SPIR-V reader, which runs these steps:
	// 1. SPIRV-Tools split-combined-image-sampler: every combined image sampler becomes a new sampler variable
	//    followed by a new image variable, both with the original binding and with IDs above all existing IDs.
	// 2. SPIRV-Tools resolve-binding-conflicts: per group, the resources statically used by the entry point are
	//    sorted by binding, then samplers after non-samplers, then by ID, and every binding which is not greater
	//    than the previous one becomes the previous binding + 1.
	// 3. tint RemapSamplers: any sampler which still shares its binding point with another variable, used or not,
	//    is moved to the highest binding in its group + 1, in declaration order.
	resolved_bindings.clear();
	resolved_sampler_bindings.clear();

	unordered_set<VariableID> active;
	if (!ir.is_library_module)
		active = get_active_interface_variables();

	struct Resource
	{
		uint32_t var_id;
		uint32_t group;
		uint32_t binding;
		bool sampler;      // Sampler-like resources sort after textures with the same binding.
		bool sampler_part; // The sampler half of a combined image sampler.
		uint32_t order;    // ID of the variable after splitting combined image samplers.
		bool active;
	};
	SmallVector<Resource> resources;

	uint32_t split_id = uint32_t(ir.ids.size());
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    if (var.storage == StorageClassFunction || var.storage == StorageClassInput ||
		        var.storage == StorageClassOutput || var.storage == StorageClassPushConstant)
			    return;
		    if (!has_decoration(var.self, DecorationDescriptorSet) || !has_decoration(var.self, DecorationBinding))
			    return;

		    const SPIRType *type = &get_variable_data_type(var);
		    while (!type->array.empty())
			    type = &this->get<SPIRType>(type->parent_type);

		    uint32_t group = get_decoration(var.self, DecorationDescriptorSet);
		    uint32_t binding = get_decoration(var.self, DecorationBinding);
		    bool is_active = ir.is_library_module || active.count(var.self) != 0;

		    if (type->basetype == SPIRType::SampledImage)
		    {
			    uint32_t sampler_order = split_id++;
			    uint32_t image_order = split_id++;
			    resources.push_back({ var.self, group, binding, true, true, sampler_order, is_active });
			    resources.push_back({ var.self, group, binding, false, false, image_order, is_active });
		    }
		    else
		    {
			    resources.push_back(
			        { var.self, group, binding, type->basetype == SPIRType::Sampler, false, var.self, is_active });
		    }
	    });

	// Step 2, resolve conflicts between the resources used by the entry point.
	unordered_map<uint32_t, SmallVector<Resource *>> groups;
	for (auto &res : resources)
		if (res.active)
			groups[res.group].push_back(&res);

	for (auto &group : groups)
	{
		auto &list = group.second;
		stable_sort(list.begin(), list.end(),
		            [](const Resource *a, const Resource *b)
		            {
			            if (a->binding != b->binding)
				            return a->binding < b->binding;
			            if (a->sampler != b->sampler)
				            return b->sampler;
			            return a->order < b->order;
		            });

		for (size_t i = 1; i < list.size(); i++)
			if (list[i]->binding <= list[i - 1]->binding)
				list[i]->binding = list[i - 1]->binding + 1;
	}

	// Step 3, move samplers which still share a binding point.
	unordered_map<uint32_t, uint32_t> max_binding;
	unordered_map<uint64_t, uint32_t> binding_use_count;
	for (auto &res : resources)
	{
		auto &m = max_binding[res.group];
		m = max(m, res.binding);
		binding_use_count[(uint64_t(res.group) << 32) | res.binding]++;
	}

	for (auto &res : resources)
	{
		if (res.sampler && binding_use_count[(uint64_t(res.group) << 32) | res.binding] > 1)
			res.binding = ++max_binding[res.group];
	}

	for (auto &res : resources)
		(res.sampler_part ? resolved_sampler_bindings : resolved_bindings)[res.var_id] = res.binding;
}

void CompilerWGSL::emit_buffer_block(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);
	if (!type.array.empty())
		SPIRV_CROSS_THROW("Arrays of buffers are not supported in WGSL.");

	bool ssbo = var.storage == StorageClassStorageBuffer ||
	            ir.meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock);

	add_resource_name(var.self);

	string space;
	if (ssbo)
	{
		auto flags = ir.get_buffer_block_flags(var);
		bool readonly = flags.get(DecorationNonWritable);
		space = readonly ? "storage, read" : "storage, read_write";
	}
	else
		space = "uniform";

	statement(binding_attributes(var.self), "var<", space, "> ", to_name(var.self), " : ",
	          type_to_glsl(type, var.self), ";");
}

void CompilerWGSL::emit_push_constant_block(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);
	add_resource_name(var.self);

	uint32_t group = wgsl_options.push_constant_group;
	uint32_t binding = wgsl_options.push_constant_binding;
	if (has_decoration(var.self, DecorationDescriptorSet))
		group = get_decoration(var.self, DecorationDescriptorSet);
	if (has_decoration(var.self, DecorationBinding))
		binding = get_decoration(var.self, DecorationBinding);

	statement("@group(", group, ") @binding(", binding, ") var<uniform> ", to_name(var.self), " : ",
	          type_to_glsl(type, var.self), ";");
}

void CompilerWGSL::emit_uniform(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);
	if (!type.array.empty())
		SPIRV_CROSS_THROW("Arrays of textures and samplers are not supported in WGSL.");

	add_resource_name(var.self);
	auto name = to_name(var.self);

	switch (type.basetype)
	{
	case SPIRType::SampledImage:
		statement(binding_attributes(var.self), "var ", name, " : ", type_to_glsl(type, var.self), ";");
		statement(binding_attributes(var.self, true), "var ", name,
		          "_sampler : ", is_comparison_sampler(var.self) ? "sampler_comparison" : "sampler", ";");
		break;

	case SPIRType::Image:
	case SPIRType::Sampler:
		statement(binding_attributes(var.self), "var ", name, " : ", type_to_glsl(type, var.self), ";");
		break;

	default:
		SPIRV_CROSS_THROW("Uniform type is not supported in WGSL.");
	}
}

void CompilerWGSL::emit_resources()
{
	auto &execution = get_entry_point();

	replace_illegal_names();

	emit_constants_and_structs();

	bool emitted = false;

	// Output UBOs and SSBOs
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    auto &type = this->get<SPIRType>(var.basetype);

		    bool is_block_storage = type.storage == StorageClassStorageBuffer || type.storage == StorageClassUniform;
		    bool has_block_flags = ir.meta[type.self].decoration.decoration_flags.get(DecorationBlock) ||
		                           ir.meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock);

		    if (var.storage != StorageClassFunction && type.pointer && is_block_storage && !is_hidden_variable(var) &&
		        has_block_flags)
		    {
			    emit_buffer_block(var);
			    emitted = true;
		    }
	    });

	// Output push constant blocks
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    auto &type = this->get<SPIRType>(var.basetype);
		    if (var.storage != StorageClassFunction && type.pointer && type.storage == StorageClassPushConstant &&
		        !is_hidden_variable(var))
		    {
			    emit_push_constant_block(var);
			    emitted = true;
		    }
	    });

	// Output textures and samplers.
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    auto &type = this->get<SPIRType>(var.basetype);
		    if (var.storage != StorageClassFunction && !is_builtin_variable(var) && !var.remapped_variable &&
		        type.pointer && type.storage == StorageClassUniformConstant && !is_hidden_variable(var))
		    {
			    emit_uniform(var);
			    emitted = true;
		    }
	    });

	if (emitted)
		statement("");
	emitted = false;

	// Stage inputs and outputs are declared as private variables.
	unordered_set<uint32_t> emitted_builtins;
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    auto &type = this->get<SPIRType>(var.basetype);
		    if ((var.storage != StorageClassInput && var.storage != StorageClassOutput) || !type.pointer ||
		        !interface_variable_exists_in_entry_point(var.self))
			    return;

		    auto &active = var.storage == StorageClassInput ? active_input_builtins : active_output_builtins;
		    if (has_decoration(var.self, DecorationBuiltIn))
		    {
			    auto builtin = BuiltIn(get_decoration(var.self, DecorationBuiltIn));
			    uint32_t key = (uint32_t(var.storage) << 16) | uint32_t(builtin);
			    if (!active.get(builtin) || emitted_builtins.count(key))
				    return;
			    emitted_builtins.insert(key);
			    if (builtin == BuiltInClipDistance)
				    clip_distance_count =
				        get_variable_data_type(var).array.empty() ? 1 : get_variable_data_type(var).array.back();
			    statement("var<private> ", builtin_to_glsl(builtin, var.storage), " : ",
			              type_to_glsl(get_variable_data_type(var)), ";");
			    emitted = true;
		    }
		    else if (is_builtin_type(get_variable_data_type(var)))
		    {
			    auto &block_type = get_variable_data_type(var);
			    for (uint32_t i = 0; i < uint32_t(block_type.member_types.size()); i++)
			    {
				    if (!has_member_decoration(block_type.self, i, DecorationBuiltIn))
					    continue;
				    auto builtin = BuiltIn(get_member_decoration(block_type.self, i, DecorationBuiltIn));
				    uint32_t key = (uint32_t(var.storage) << 16) | uint32_t(builtin);
				    if (!active.get(builtin) || emitted_builtins.count(key))
					    continue;
				    emitted_builtins.insert(key);
				    auto &member_type = this->get<SPIRType>(block_type.member_types[i]);
				    if (builtin == BuiltInClipDistance)
					    clip_distance_count = member_type.array.empty() ? 1 : member_type.array.back();
				    statement("var<private> ", builtin_to_glsl(builtin, var.storage), " : ", type_to_glsl(member_type),
				              ";");
				    emitted = true;
			    }
		    }
		    else
		    {
			    add_resource_name(var.self);
			    statement("var<private> ", to_name(var.self), " : ",
			              type_to_glsl(get_variable_data_type(var), var.self), ";");
			    emitted = true;
		    }
	    });

	// Global variables.
	for (auto global : global_variables)
	{
		auto &var = get<SPIRVariable>(global);
		if (is_hidden_variable(var, true))
			continue;

		if (var.storage == StorageClassPrivate || var.storage == StorageClassWorkgroup)
		{
			if (var.storage == StorageClassWorkgroup && execution.model != ExecutionModelGLCompute)
				SPIRV_CROSS_THROW("Workgroup variables are only supported in compute shaders.");

			add_resource_name(var.self);
			statement(variable_decl(var), ";");
			emitted = true;
		}
	}

	if (emitted)
		statement("");
}

// Entry point wrapper

string CompilerWGSL::interpolation_attributes(const Bitset &flags, const SPIRType &type, StorageClass storage,
                                              bool flat_required)
{
	auto &execution = get_entry_point();
	bool interpolated = (execution.model == ExecutionModelVertex && storage == StorageClassOutput) ||
	                    (execution.model == ExecutionModelFragment && storage == StorageClassInput);
	if (!interpolated)
		return "";

	if (flat_required || flags.get(DecorationFlat) || type.basetype == SPIRType::Int || type.basetype == SPIRType::UInt)
		return "@interpolate(flat) ";

	const char *interp = flags.get(DecorationNoPerspective) ? "linear" : "perspective";
	if (flags.get(DecorationCentroid))
		return join("@interpolate(", interp, ", centroid) ");
	else if (flags.get(DecorationSample))
		return join("@interpolate(", interp, ", sample) ");
	else if (flags.get(DecorationNoPerspective))
		return "@interpolate(linear) ";
	return "";
}

void CompilerWGSL::add_stage_io_member(SmallVector<StageIOMember> &members, const string &lhs, const string &name,
                                       uint32_t type_id, uint32_t &location, const Bitset &flags, StorageClass storage,
                                       bool flat_required)
{
	auto &type = get<SPIRType>(type_id);

	if (!type.array.empty())
	{
		if (!type.array_size_literal.back())
			SPIRV_CROSS_THROW("Stage I/O arrays must have a literal size in WGSL.");
		for (uint32_t i = 0; i < type.array.back(); i++)
			add_stage_io_member(members, join(lhs, "[", i, "]"), join(name, "_", i), type.parent_type, location, flags,
			                    storage, flat_required);
		return;
	}

	if (type.basetype == SPIRType::Struct)
	{
		for (uint32_t i = 0; i < uint32_t(type.member_types.size()); i++)
		{
			if (has_member_decoration(type.self, i, DecorationLocation))
				location = get_member_decoration(type.self, i, DecorationLocation);
			Bitset member_flags = flags;
			member_flags.merge_or(get_member_decoration_bitset(type.self, i));
			auto member_name = to_member_name(type, i);
			add_stage_io_member(members, join(lhs, ".", member_name), join(name, "_", member_name),
			                    type.member_types[i], location, member_flags, storage, flat_required);
		}
		return;
	}

	if (type.columns > 1)
	{
		for (uint32_t i = 0; i < type.columns; i++)
			add_stage_io_member(members, join(lhs, "[", i, "]"), join(name, "_", i), type.parent_type, location, flags,
			                    storage, flat_required);
		return;
	}

	if (type.basetype == SPIRType::Boolean)
		SPIRV_CROSS_THROW("Boolean stage inputs and outputs are not supported in WGSL.");

	StageIOMember member;
	member.lhs = lhs;
	member.name = name;
	member.type_id = type_id;
	member.wgsl_type = type_to_glsl(type);
	member.sort_key = location * 2;
	member.attributes = join("@location(", location, ") ");
	member.attributes += interpolation_attributes(flags, type, storage, flat_required);
	members.push_back(std::move(member));
	location++;
}

void CompilerWGSL::add_stage_io_builtin(SmallVector<StageIOMember> &members, BuiltIn builtin, uint32_t type_id,
                                        StorageClass storage, bool invariant)
{
	auto &execution = get_entry_point();
	auto name = wgsl_builtin_name(builtin, storage);
	if (name.empty())
	{
		// Some builtins are written but have no equivalent in WGSL. They are ignored.
		if (builtin == BuiltInPointSize)
		{
			warn(WGSL_WARNING_IGNORED_BUILTIN, "gl_PointSize is written, but ignored since WGSL always renders points with size 1.");
			return;
		}
		if (builtin == BuiltInPosition && storage == StorageClassInput)
			return;
		if (builtin == BuiltInClipDistance)
		{
			warn(WGSL_WARNING_IGNORED_BUILTIN,
			     storage == StorageClassInput ? "gl_ClipDistance input is not available in WGSL and reads as zero." :
			                                    "gl_ClipDistance is written, but ignored outside of vertex shaders.");
			return;
		}
		SPIRV_CROSS_THROW(join("Builtin ", builtin_to_glsl(builtin, storage), " is not supported in WGSL."));
	}

	if (builtin == BuiltInClipDistance)
		require_enable(requires_clip_distances);
	if (builtin == BuiltInSubgroupSize || builtin == BuiltInSubgroupLocalInvocationId)
		require_enable(requires_subgroups);

	(void)execution;
	auto &type = get<SPIRType>(type_id);

	StageIOMember member;
	member.name = builtin_to_glsl(builtin, storage);
	member.lhs = member.name;
	member.type_id = type_id;
	member.wgsl_type = wgsl_builtin_type(builtin);
	member.sort_key = 0x10000 + uint32_t(builtin);
	member.attributes = join("@builtin(", name, ") ");
	if (invariant && builtin == BuiltInPosition)
		member.attributes += "@invariant ";

	// Sample mask is an array in SPIR-V.
	if (builtin == BuiltInSampleMask && !type.array.empty())
	{
		member.lhs += "[0]";
		member.type_id = type.parent_type;
	}

	members.push_back(std::move(member));
}

void CompilerWGSL::collect_stage_io(StorageClass storage, SmallVector<StageIOMember> &members)
{
	auto &active = storage == StorageClassInput ? active_input_builtins : active_output_builtins;
	unordered_set<uint32_t> seen_builtins;

	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    auto &type = this->get<SPIRType>(var.basetype);
		    if (var.storage != storage || !type.pointer || !interface_variable_exists_in_entry_point(var.self))
			    return;

		    auto &data_type = get_variable_data_type(var);
		    bool invariant = has_decoration(var.self, DecorationInvariant);

		    if (has_decoration(var.self, DecorationBuiltIn))
		    {
			    auto builtin = BuiltIn(get_decoration(var.self, DecorationBuiltIn));
			    if (active.get(builtin) && !seen_builtins.count(builtin))
			    {
				    seen_builtins.insert(builtin);
				    add_stage_io_builtin(members, builtin, get_variable_data_type_id(var), storage, invariant);
			    }
		    }
		    else if (is_builtin_type(data_type))
		    {
			    for (uint32_t i = 0; i < uint32_t(data_type.member_types.size()); i++)
			    {
				    if (!has_member_decoration(data_type.self, i, DecorationBuiltIn))
					    continue;
				    auto builtin = BuiltIn(get_member_decoration(data_type.self, i, DecorationBuiltIn));
				    if (!active.get(builtin) || seen_builtins.count(builtin))
					    continue;
				    seen_builtins.insert(builtin);
				    bool member_invariant = invariant || has_member_decoration(data_type.self, i, DecorationInvariant);
				    add_stage_io_builtin(members, builtin, data_type.member_types[i], storage, member_invariant);
			    }
		    }
		    else
		    {
			    uint32_t location = get_decoration(var.self, DecorationLocation);
			    auto flags = get_decoration_bitset(var.self);
			    if (storage == StorageClassOutput && get_entry_point().model == ExecutionModelFragment &&
			        has_decoration(var.self, DecorationIndex))
			    {
				    require_enable(requires_dual_source_blending);
			    }
			    auto name = to_name(var.self);
			    size_t first = members.size();
			    add_stage_io_member(members, name, name, get_variable_data_type_id(var), location, flags, storage,
			                        false);

			    if (requires_dual_source_blending && has_decoration(var.self, DecorationIndex))
			    {
				    uint32_t index = get_decoration(var.self, DecorationIndex);
				    for (size_t i = first; i < members.size(); i++)
				    {
					    members[i].attributes += join("@blend_src(", index, ") ");
					    members[i].sort_key += index;
				    }
			    }
		    }
	    });

	stable_sort(members.begin(), members.end(),
	            [](const StageIOMember &a, const StageIOMember &b) { return a.sort_key < b.sort_key; });

	for (size_t i = 1; i < members.size(); i++)
	{
		if (members[i].sort_key < 0x10000 && members[i].sort_key == members[i - 1].sort_key)
			SPIRV_CROSS_THROW("Multiple stage inputs or outputs at the same location (Component decoration) are not "
			                  "supported in WGSL.");
	}
}

void CompilerWGSL::emit_entry_point_wrapper()
{
	auto &execution = get_entry_point();

	SmallVector<StageIOMember> inputs, outputs;
	collect_stage_io(StorageClassInput, inputs);
	collect_stage_io(StorageClassOutput, outputs);

	if (execution.model == ExecutionModelVertex)
	{
		bool has_position = false;
		for (auto &m : outputs)
			if (m.attributes.find("@builtin(position)") != string::npos)
				has_position = true;
		if (!has_position)
		{
			StageIOMember member;
			member.name = "gl_Position";
			member.lhs = "";
			member.wgsl_type = "vec4f";
			member.attributes = "@builtin(position) ";
			member.sort_key = 0x10000;
			outputs.push_back(std::move(member));
		}
	}

	if (!inputs.empty())
	{
		statement("struct SPIRV_Cross_Input");
		begin_scope();
		for (auto &m : inputs)
			statement(m.attributes, m.name, " : ", m.wgsl_type, ",");
		end_scope();
		statement("");
	}

	if (!outputs.empty())
	{
		statement("struct SPIRV_Cross_Output");
		begin_scope();
		for (auto &m : outputs)
			statement(m.attributes, m.name, " : ", m.wgsl_type, ",");
		end_scope();
		statement("");
	}

	switch (execution.model)
	{
	case ExecutionModelVertex:
		statement("@vertex");
		break;
	case ExecutionModelFragment:
		statement("@fragment");
		break;
	case ExecutionModelGLCompute:
	{
		SpecializationConstant wg_x, wg_y, wg_z;
		get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);
		string x = wg_x.id ? to_name(wg_x.id) : convert_to_string(execution.workgroup_size.x);
		string y = wg_y.id ? to_name(wg_y.id) : convert_to_string(execution.workgroup_size.y);
		string z = wg_z.id ? to_name(wg_z.id) : convert_to_string(execution.workgroup_size.z);
		statement("@compute @workgroup_size(", x, ", ", y, ", ", z, ")");
		break;
	}
	default:
		break;
	}

	string decl = join("fn ", get_entry_point_wrapper_name(), "(");
	if (!inputs.empty())
		decl += "stage_input : SPIRV_Cross_Input";
	decl += ")";
	if (!outputs.empty())
		decl += " -> SPIRV_Cross_Output";
	statement(decl);
	begin_scope();

	for (auto &m : inputs)
	{
		auto &type = get<SPIRType>(m.type_id);
		string value = join("stage_input.", m.name);
		if (type_to_glsl(type) != m.wgsl_type)
			value = join(type_to_glsl(type), "(", value, ")");
		statement(m.lhs, " = ", value, ";");
	}

	statement(get_inner_entry_point_name(), "();");

	if (!outputs.empty())
	{
		statement("var stage_output : SPIRV_Cross_Output;");
		for (auto &m : outputs)
		{
			if (m.lhs.empty())
				continue;
			auto &type = get<SPIRType>(m.type_id);
			string value = m.lhs;
			if (type_to_glsl(type) != m.wgsl_type)
				value = join(m.wgsl_type, "(", value, ")");
			statement("stage_output.", m.name, " = ", value, ";");
		}
		statement("return stage_output;");
	}

	end_scope();
}

// Expressions

string CompilerWGSL::splat(const SPIRType &type, const string &scalar_literal)
{
	if (type.vecsize == 1)
		return scalar_literal;
	return join(type_to_glsl(type), "(", scalar_literal, ")");
}

string CompilerWGSL::to_unsigned_expression(uint32_t id, uint32_t vecsize)
{
	auto &type = expression_type(id);
	auto target = type;
	target.basetype = SPIRType::UInt;
	target.vecsize = vecsize;
	target.columns = 1;

	string expr = to_unpacked_expression(id);
	if (type.basetype != SPIRType::UInt)
		expr = join(type_to_glsl(target), "(", expr, ")");
	else if (type.vecsize != vecsize)
		expr = join(type_to_glsl(target), "(", expr, ")");
	return expr;
}

// In WGSL, "a < b, c > d" in an argument list is parsed as a template list if a is an identifier.
static bool ends_with_identifier(const string &expr)
{
	size_t i = expr.size();
	while (i > 0 && (isalnum(static_cast<unsigned char>(expr[i - 1])) || expr[i - 1] == '_'))
		i--;
	return i < expr.size() && !isdigit(static_cast<unsigned char>(expr[i]));
}

static string component_expression(const string &enclosed, uint32_t vecsize, uint32_t start, uint32_t count)
{
	if (vecsize == 1)
		return enclosed;
	if (start == 0 && count == vecsize)
		return enclosed;
	return join(enclosed, ".", string(swizzle_components + start, count));
}

string CompilerWGSL::to_function_name(const TextureFunctionNameArguments &args)
{
	bool implicit_lod = implicit_lod_allowed();
	if (implicit_lod && !args.base.is_fetch && !args.base.is_gather && !args.lod && !args.has_grad)
		require_enable(uses_implicit_derivatives);
	if (args.base.is_fetch)
		return "textureLoad";
	if (args.base.is_gather)
		return args.has_dref ? "textureGatherCompare" : "textureGather";
	if (args.has_dref)
		return (args.lod || args.has_grad || !implicit_lod) ? "textureSampleCompareLevel" : "textureSampleCompare";
	if (args.has_grad)
		return "textureSampleGrad";
	if (args.lod)
		return "textureSampleLevel";
	if (args.has_bias && implicit_lod)
		return "textureSampleBias";
	return implicit_lod ? "textureSample" : "textureSampleLevel";
}

string CompilerWGSL::to_function_args(const TextureFunctionArguments &args, bool *p_forward)
{
	if (args.dref && !args.base.is_gather && !args.base.is_fetch)
	{
		auto *lod_constant = args.lod ? maybe_get<SPIRConstant>(args.lod) : nullptr;
		bool zero_lod = lod_constant && !lod_constant->specialization && lod_constant->scalar_f32() == 0.0f;
		if (args.grad_x || (args.lod && !zero_lod))
		{
			warn(WGSL_WARNING_DEPTH_COMPARE_LOD,
			     join("Depth comparison with ", args.grad_x ? "explicit gradients" : "an explicit LOD", " ",
			          warning_location(), " samples LOD 0, since WGSL only supports depth comparisons at LOD 0."));
		}
	}

	if (args.bias && !implicit_lod_allowed())
	{
		warn(WGSL_WARNING_IGNORED_BIAS,
		     join("LOD bias ", warning_location(), " is ignored, since WGSL only supports bias in fragment shaders."));
	}

	auto &imgtype = *args.base.imgtype;
	uint32_t img = args.base.img;
	bool implicit_lod = implicit_lod_allowed();
	bool depth = is_depth_texture(img);

	uint32_t dims = 2;
	switch (imgtype.image.dim)
	{
	case Dim1D:
		dims = 1;
		break;
	case Dim3D:
	case DimCube:
		dims = 3;
		break;
	default:
		dims = 2;
		break;
	}

	bool forward = should_forward(args.coord);
	auto &coord_type = expression_type(args.coord);
	string coord_expr = to_enclosed_unpacked_expression(args.coord);

	SmallVector<string> arglist;

	if (args.base.is_fetch)
	{
		arglist.push_back(to_image_expression(img));
		string coords = component_expression(coord_expr, coord_type.vecsize, 0, dims);
		if (args.offset)
		{
			coords = join(coords, " + ", to_enclosed_expression(args.offset));
			forward = forward && should_forward(args.offset);
		}
		arglist.push_back(coords);
		if (imgtype.image.arrayed)
			arglist.push_back(component_expression(coord_expr, coord_type.vecsize, dims, 1));
		if (imgtype.image.ms)
		{
			arglist.push_back(to_expression(args.sample));
			forward = forward && should_forward(args.sample);
		}
		else if (imgtype.image.sampled != 2)
		{
			if (args.lod)
			{
				arglist.push_back(to_expression(args.lod));
				forward = forward && should_forward(args.lod);
			}
			else
				arglist.push_back("0");
		}
		*p_forward = forward;
		return merge(arglist);
	}

	if (args.base.is_gather && !args.dref && !depth)
	{
		if (args.component)
			arglist.push_back(to_expression(args.component));
		else
			arglist.push_back("0");
	}

	arglist.push_back(to_image_expression(img));
	arglist.push_back(to_sampler_expression(img));

	// Coordinates.
	if (get<SPIRType>(imgtype.image.type).basetype != SPIRType::Float &&
	    get<SPIRType>(imgtype.image.type).basetype != SPIRType::Half)
	{
		SPIRV_CROSS_THROW("WGSL does not support sampling integer textures, only texelFetch().");
	}

	if (imgtype.image.dim == Dim1D &&
	    (args.lod || args.bias || args.grad_x || args.offset || args.dref || args.base.is_gather || !implicit_lod))
	{
		SPIRV_CROSS_THROW("WGSL only supports textureSample() without LOD, bias, gradients or offsets on 1D textures.");
	}

	string q;
	if (args.base.is_proj)
		q = component_expression(coord_expr, coord_type.vecsize, args.coord_components - 1, 1);

	string coords = component_expression(coord_expr, coord_type.vecsize, 0, dims);
	if (!q.empty())
		coords = join(coords, " / ", q);
	arglist.push_back(coords);

	if (imgtype.image.arrayed)
		arglist.push_back(join("i32(round(", component_expression(coord_expr, coord_type.vecsize, dims, 1), "))"));

	if (args.dref)
	{
		string dref = to_enclosed_unpacked_expression(args.dref);
		if (!q.empty())
			dref = join(dref, " / ", q);
		arglist.push_back(dref);
		forward = forward && should_forward(args.dref);
	}
	else if (args.base.is_gather)
	{
		// Nothing more.
	}
	else if (args.grad_x && args.grad_y)
	{
		arglist.push_back(to_unpacked_expression(args.grad_x));
		arglist.push_back(to_unpacked_expression(args.grad_y));
		forward = forward && should_forward(args.grad_x) && should_forward(args.grad_y);
	}
	else if (args.lod)
	{
		if (depth)
			arglist.push_back(join("i32(", to_unpacked_expression(args.lod), ")"));
		else
			arglist.push_back(to_unpacked_expression(args.lod));
		forward = forward && should_forward(args.lod);
	}
	else if (args.bias && implicit_lod)
	{
		arglist.push_back(to_unpacked_expression(args.bias));
		forward = forward && should_forward(args.bias);
	}
	else if (!implicit_lod)
	{
		arglist.push_back(depth ? "0" : "0.0f");
	}

	if (args.offset && imgtype.image.dim != DimCube)
	{
		if (args.has_array_offsets)
			SPIRV_CROSS_THROW("textureGatherOffsets is not supported in WGSL.");
		arglist.push_back(to_expression(args.offset));
	}

	if (args.min_lod)
		SPIRV_CROSS_THROW("Min LOD is not supported in WGSL.");
	if (args.sparse_texel)
		SPIRV_CROSS_THROW("Sparse texture feedback is not supported in WGSL.");

	*p_forward = forward;
	return merge(arglist);
}

void CompilerWGSL::emit_image_query(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);
	uint32_t result_type_id = ops[0];
	uint32_t id = ops[1];
	uint32_t img = ops[2];
	auto &result_type = get<SPIRType>(result_type_id);
	auto &type = expression_type(img);
	string img_expr = to_image_expression(img);

	auto scalar_type = result_type;
	scalar_type.vecsize = 1;
	string scalar = type_to_glsl(scalar_type);

	string expr;
	switch (opcode)
	{
	case OpImageQuerySizeLod:
	case OpImageQuerySize:
	{
		uint32_t dims = 2;
		switch (type.image.dim)
		{
		case Dim1D:
			dims = 1;
			break;
		case Dim3D:
			dims = 3;
			break;
		default:
			dims = 2;
			break;
		}

		string dim_expr;
		if (opcode == OpImageQuerySizeLod)
			dim_expr = join("textureDimensions(", img_expr, ", ", to_unpacked_expression(ops[3]), ")");
		else
			dim_expr = join("textureDimensions(", img_expr, ")");

		auto dim_type = result_type;
		dim_type.vecsize = dims;
		dim_expr = join(type_to_glsl(dim_type), "(", dim_expr, ")");

		if (type.image.arrayed)
			expr = join(type_to_glsl(result_type), "(", dim_expr, ", ", scalar, "(textureNumLayers(", img_expr, ")))");
		else
			expr = dim_expr;
		break;
	}

	case OpImageQueryLevels:
		expr = join(scalar, "(textureNumLevels(", img_expr, "))");
		break;

	case OpImageQuerySamples:
		expr = join(scalar, "(textureNumSamples(", img_expr, "))");
		break;

	default:
		SPIRV_CROSS_THROW("Unsupported image query.");
	}

	emit_op(result_type_id, id, expr, true);
}

void CompilerWGSL::emit_atomic_op(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);

	if (opcode == OpAtomicStore)
	{
		uint32_t ptr = ops[0];
		uint32_t value = ops[3];
		auto &ptr_type = expression_type(ptr);
		auto &value_type = expression_type(value);
		string value_expr = to_unpacked_expression(value);
		if (value_type.basetype != ptr_type.basetype)
			value_expr = bitcast_expression(get_pointee_type(ptr_type), value_type.basetype, value_expr);
		statement("atomicStore(&", to_expression(ptr), ", ", value_expr, ");");
		flush_all_atomic_capable_variables();
		register_write(ptr);
		return;
	}

	uint32_t result_type = ops[0];
	uint32_t id = ops[1];
	uint32_t ptr = ops[2];
	auto &ptr_type = get_pointee_type(expression_type(ptr));
	if (ptr_type.basetype != SPIRType::Int && ptr_type.basetype != SPIRType::UInt)
		SPIRV_CROSS_THROW("WGSL only supports atomics on 32-bit integers.");
	string ptr_expr = join("&", to_expression(ptr));

	auto value_expr = [&](uint32_t value) -> string
	{
		auto &value_type = expression_type(value);
		string expr = to_unpacked_expression(value);
		if (value_type.basetype != ptr_type.basetype)
			expr = bitcast_expression(ptr_type, value_type.basetype, expr);
		return expr;
	};

	string expr;
	switch (opcode)
	{
	case OpAtomicLoad:
		expr = join("atomicLoad(", ptr_expr, ")");
		break;
	case OpAtomicExchange:
		expr = join("atomicExchange(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	case OpAtomicCompareExchange:
	case OpAtomicCompareExchangeWeak:
		if (opcode == OpAtomicCompareExchange)
		{
			warn(WGSL_WARNING_WEAK_COMPARE_EXCHANGE,
			     join("Compare-exchange ", warning_location(),
			          " is emitted as atomicCompareExchangeWeak(), which may fail spuriously."));
		}
		expr = join("atomicCompareExchangeWeak(", ptr_expr, ", ", value_expr(ops[7]), ", ", value_expr(ops[6]),
		            ").old_value");
		break;
	case OpAtomicIIncrement:
		expr = join("atomicAdd(", ptr_expr, ", ", ptr_type.basetype == SPIRType::Int ? "1i" : "1u", ")");
		break;
	case OpAtomicIDecrement:
		expr = join("atomicSub(", ptr_expr, ", ", ptr_type.basetype == SPIRType::Int ? "1i" : "1u", ")");
		break;
	case OpAtomicIAdd:
		expr = join("atomicAdd(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	case OpAtomicISub:
		expr = join("atomicSub(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	case OpAtomicSMin:
	case OpAtomicUMin:
		expr = join("atomicMin(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	case OpAtomicSMax:
	case OpAtomicUMax:
		expr = join("atomicMax(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	case OpAtomicAnd:
		expr = join("atomicAnd(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	case OpAtomicOr:
		expr = join("atomicOr(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	case OpAtomicXor:
		expr = join("atomicXor(", ptr_expr, ", ", value_expr(ops[5]), ")");
		break;
	default:
		SPIRV_CROSS_THROW("Unsupported atomic operation.");
	}

	auto &out_type = get<SPIRType>(result_type);
	if (out_type.basetype != ptr_type.basetype)
		expr = bitcast_expression(out_type, ptr_type.basetype, expr);

	forced_temporaries.insert(id);
	emit_op(result_type, id, expr, false);
	flush_all_atomic_capable_variables();
	register_write(ptr);
}

void CompilerWGSL::emit_shift_op(uint32_t result_type, uint32_t id, uint32_t op0, uint32_t op1, const char *op,
                                 SPIRType::BaseType input_type)
{
	auto &out_type = get<SPIRType>(result_type);
	auto &type0 = expression_type(op0);

	string lhs = to_enclosed_unpacked_expression(op0);
	if (type0.basetype != input_type)
	{
		auto expected = type0;
		expected.basetype = input_type;
		lhs = bitcast_expression(expected, type0.basetype, lhs);
	}

	string rhs = to_unsigned_expression(op1, type0.vecsize);
	if (needs_enclose_expression(rhs))
		rhs = enclose_expression(rhs);

	string expr = join(lhs, " ", op, " ", rhs);
	if (out_type.basetype != input_type)
		expr = bitcast_expression(out_type, input_type, expr);

	emit_op(result_type, id, expr, should_forward(op0) && should_forward(op1));
	inherit_expression_dependencies(id, op0);
	inherit_expression_dependencies(id, op1);
}

void CompilerWGSL::emit_vector_compare(uint32_t result_type, uint32_t id, uint32_t op0, uint32_t op1, const char *op,
                                       SPIRType::BaseType input_type, bool unordered, bool sign_invariant)
{
	if (input_type == SPIRType::Unknown)
	{
		string lhs = to_enclosed_unpacked_expression(op0);
		string expr = join(lhs, " ", op, " ", to_enclosed_unpacked_expression(op1));
		if (unordered)
			expr = join("!(", expr, ")");
		else if (strcmp(op, "<") == 0 && ends_with_identifier(lhs))
			expr = join("(", expr, ")");
		emit_op(result_type, id, expr, should_forward(op0) && should_forward(op1));
		inherit_expression_dependencies(id, op0);
		inherit_expression_dependencies(id, op1);
	}
	else
	{
		emit_binary_op_cast(result_type, id, op0, op1, op, input_type, sign_invariant, false);
		if (strcmp(op, "<") == 0)
		{
			// "a < b, c > d" in an argument list is parsed as a template list in WGSL.
			auto *e = maybe_get<SPIRExpression>(id);
			auto cmp = e ? e->expression.find(" < ") : string::npos;
			if (e && cmp != string::npos && ends_with_identifier(e->expression.substr(0, cmp)) &&
			    !(e->expression.front() == '(' && e->expression.back() == ')'))
			{
				e->expression = join("(", e->expression, ")");
			}
		}
	}
}

void CompilerWGSL::emit_subgroup_op(const Instruction &i)
{
	const uint32_t *ops = stream(i);
	auto op = static_cast<Op>(i.op);
	uint32_t result_type = ops[0];
	uint32_t id = ops[1];

	if (evaluate_constant_u32(ops[2]) != ScopeSubgroup)
		SPIRV_CROSS_THROW("Only subgroup scope is supported in WGSL.");

	require_enable(requires_subgroups);

	auto emit_group_op = [&](const char *reduce, const char *inclusive, const char *exclusive)
	{
		auto operation = static_cast<GroupOperation>(ops[3]);
		const char *func = nullptr;
		switch (operation)
		{
		case GroupOperationReduce:
			func = reduce;
			break;
		case GroupOperationInclusiveScan:
			func = inclusive;
			break;
		case GroupOperationExclusiveScan:
			func = exclusive;
			break;
		default:
			break;
		}
		if (!func)
			SPIRV_CROSS_THROW("Unsupported subgroup group operation in WGSL.");
		emit_unary_func_op(result_type, id, ops[4], func);
	};

	auto int_type = to_signed_basetype(32);
	auto uint_type = to_unsigned_basetype(32);

	switch (op)
	{
	case OpGroupNonUniformElect:
		emit_op(result_type, id, "subgroupElect()", false);
		break;

	case OpGroupNonUniformAll:
		emit_unary_func_op(result_type, id, ops[3], "subgroupAll");
		break;

	case OpGroupNonUniformAny:
		emit_unary_func_op(result_type, id, ops[3], "subgroupAny");
		break;

	case OpGroupNonUniformBallot:
		emit_unary_func_op(result_type, id, ops[3], "subgroupBallot");
		break;

	case OpGroupNonUniformBroadcastFirst:
		emit_unary_func_op(result_type, id, ops[3], "subgroupBroadcastFirst");
		break;

	case OpGroupNonUniformBroadcast:
	{
		// subgroupBroadcast() requires a constant lane index.
		bool constant_lane = maybe_get<SPIRConstant>(ops[4]) != nullptr;
		string expr = join(constant_lane ? "subgroupBroadcast(" : "subgroupShuffle(", to_unpacked_expression(ops[3]),
		                   ", ", to_unsigned_expression(ops[4], 1), ")");
		emit_op(result_type, id, expr, false);
		break;
	}

	case OpGroupNonUniformShuffle:
	case OpGroupNonUniformShuffleXor:
	case OpGroupNonUniformShuffleUp:
	case OpGroupNonUniformShuffleDown:
	{
		const char *func = op == OpGroupNonUniformShuffle    ? "subgroupShuffle" :
		                   op == OpGroupNonUniformShuffleXor ? "subgroupShuffleXor" :
		                   op == OpGroupNonUniformShuffleUp  ? "subgroupShuffleUp" :
		                                                       "subgroupShuffleDown";
		emit_op(result_type, id,
		        join(func, "(", to_unpacked_expression(ops[3]), ", ", to_unsigned_expression(ops[4], 1), ")"), false);
		break;
	}

	case OpGroupNonUniformIAdd:
	case OpGroupNonUniformFAdd:
		emit_group_op("subgroupAdd", "subgroupInclusiveAdd", "subgroupExclusiveAdd");
		break;

	case OpGroupNonUniformIMul:
	case OpGroupNonUniformFMul:
		emit_group_op("subgroupMul", "subgroupInclusiveMul", "subgroupExclusiveMul");
		break;

	case OpGroupNonUniformSMin:
	case OpGroupNonUniformSMax:
	case OpGroupNonUniformUMin:
	case OpGroupNonUniformUMax:
	{
		if (static_cast<GroupOperation>(ops[3]) != GroupOperationReduce)
			SPIRV_CROSS_THROW("Only reductions are supported for subgroup min/max in WGSL.");
		bool is_min = op == OpGroupNonUniformSMin || op == OpGroupNonUniformUMin;
		bool is_signed = op == OpGroupNonUniformSMin || op == OpGroupNonUniformSMax;
		emit_unary_func_op_cast(result_type, id, ops[4], is_min ? "subgroupMin" : "subgroupMax",
		                        is_signed ? int_type : uint_type, is_signed ? int_type : uint_type);
		break;
	}

	case OpGroupNonUniformFMin:
	case OpGroupNonUniformFMax:
	case OpGroupNonUniformBitwiseAnd:
	case OpGroupNonUniformBitwiseOr:
	case OpGroupNonUniformBitwiseXor:
	{
		if (static_cast<GroupOperation>(ops[3]) != GroupOperationReduce)
			SPIRV_CROSS_THROW("Only reductions are supported for this subgroup operation in WGSL.");
		const char *func = op == OpGroupNonUniformFMin       ? "subgroupMin" :
		                   op == OpGroupNonUniformFMax       ? "subgroupMax" :
		                   op == OpGroupNonUniformBitwiseAnd ? "subgroupAnd" :
		                   op == OpGroupNonUniformBitwiseOr  ? "subgroupOr" :
		                                                       "subgroupXor";
		emit_unary_func_op(result_type, id, ops[4], func);
		break;
	}

	case OpGroupNonUniformLogicalAnd:
	case OpGroupNonUniformLogicalOr:
		if (static_cast<GroupOperation>(ops[3]) != GroupOperationReduce || get<SPIRType>(result_type).vecsize != 1)
			SPIRV_CROSS_THROW("Unsupported subgroup logical operation in WGSL.");
		emit_unary_func_op(result_type, id, ops[4], op == OpGroupNonUniformLogicalAnd ? "subgroupAll" : "subgroupAny");
		break;

	case OpGroupNonUniformQuadBroadcast:
		emit_op(result_type, id,
		        join("quadBroadcast(", to_unpacked_expression(ops[3]), ", ", to_unsigned_expression(ops[4], 1), ")"),
		        false);
		break;

	case OpGroupNonUniformQuadSwap:
	{
		uint32_t direction = evaluate_constant_u32(ops[4]);
		const char *func = direction == 0 ? "quadSwapX" : direction == 1 ? "quadSwapY" : "quadSwapDiagonal";
		emit_unary_func_op(result_type, id, ops[3], func);
		break;
	}

	default:
		SPIRV_CROSS_THROW("Subgroup operation is not supported in WGSL.");
	}

	register_control_dependent_expression(id);
}

void CompilerWGSL::emit_select_op(uint32_t result_type, uint32_t id, uint32_t cond, uint32_t true_value,
                                  uint32_t false_value)
{
	auto &type = get<SPIRType>(result_type);
	if (type.pointer)
		SPIRV_CROSS_THROW("Selecting pointers is not supported in WGSL.");

	if (type.columns > 1 || !type.array.empty() || type.basetype == SPIRType::Struct)
	{
		// select() only works on scalars and vectors.
		emit_uninitialized_temporary_expression(result_type, id);
		statement("if (", to_expression(cond), ")");
		begin_scope();
		statement(to_expression(id), " = ", to_unpacked_expression(true_value), ";");
		end_scope();
		statement("else");
		begin_scope();
		statement(to_expression(id), " = ", to_unpacked_expression(false_value), ";");
		end_scope();
		return;
	}

	string expr = to_ternary_expression(type, cond, true_value, false_value);
	emit_op(result_type, id, expr, should_forward(cond) && should_forward(true_value) && should_forward(false_value));
	inherit_expression_dependencies(id, cond);
	inherit_expression_dependencies(id, true_value);
	inherit_expression_dependencies(id, false_value);
}

void CompilerWGSL::emit_isnan_isinf(uint32_t result_type, uint32_t id, uint32_t op0, bool is_nan)
{
	auto &type = expression_type(op0);
	string expr;
	if (type.basetype == SPIRType::Float)
	{
		auto utype = type;
		utype.basetype = SPIRType::UInt;
		string bits = join("(bitcast<", type_to_glsl(utype), ">(", to_unpacked_expression(op0), ") & ",
		                   splat(utype, "0x7fffffffu"), ")");
		expr = join(bits, is_nan ? " > " : " == ", splat(utype, "0x7f800000u"));
	}
	else
	{
		string e = to_enclosed_unpacked_expression(op0);
		if (is_nan)
			expr = join(e, " != ", e);
		else
			expr = join("(", e, " == ", e, ") & (", e, " - ", e, " != ", e, " - ", e, ")");
	}
	emit_op(result_type, id, expr, should_forward(op0));
	inherit_expression_dependencies(id, op0);
}

void CompilerWGSL::emit_bitfield_op(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);
	uint32_t result_type = ops[0];
	uint32_t id = ops[1];
	auto &out_type = get<SPIRType>(result_type);

	string expr;
	bool forward = true;
	switch (opcode)
	{
	case OpBitFieldInsert:
		expr = join("insertBits(", to_unpacked_expression(ops[2]), ", ", to_unpacked_expression(ops[3]), ", ",
		            to_unsigned_expression(ops[4], 1), ", ", to_unsigned_expression(ops[5], 1), ")");
		forward = should_forward(ops[2]) && should_forward(ops[3]) && should_forward(ops[4]) && should_forward(ops[5]);
		for (uint32_t i = 2; i < 6; i++)
			inherit_expression_dependencies(id, ops[i]);
		break;

	case OpBitFieldSExtract:
	case OpBitFieldUExtract:
	{
		auto input_type = opcode == OpBitFieldSExtract ? SPIRType::Int : SPIRType::UInt;
		auto &base_type = expression_type(ops[2]);
		string base = to_unpacked_expression(ops[2]);
		if (base_type.basetype != input_type)
		{
			auto expected = base_type;
			expected.basetype = input_type;
			base = bitcast_expression(expected, base_type.basetype, base);
		}
		expr = join("extractBits(", base, ", ", to_unsigned_expression(ops[3], 1), ", ",
		            to_unsigned_expression(ops[4], 1), ")");
		if (out_type.basetype != input_type)
			expr = bitcast_expression(out_type, input_type, expr);
		forward = should_forward(ops[2]) && should_forward(ops[3]) && should_forward(ops[4]);
		for (uint32_t i = 2; i < 5; i++)
			inherit_expression_dependencies(id, ops[i]);
		break;
	}

	case OpBitCount:
	case OpBitReverse:
	{
		auto &in_type = expression_type(ops[2]);
		expr = join(opcode == OpBitCount ? "countOneBits(" : "reverseBits(", to_unpacked_expression(ops[2]), ")");
		if (out_type.basetype != in_type.basetype)
			expr = bitcast_expression(out_type, in_type.basetype, expr);
		forward = should_forward(ops[2]);
		inherit_expression_dependencies(id, ops[2]);
		break;
	}

	default:
		break;
	}

	emit_op(result_type, id, expr, forward);
}

string CompilerWGSL::constant_op_expression(const SPIRConstantOp &cop)
{
	switch (cop.opcode)
	{
	case OpShiftLeftLogical:
	case OpShiftRightLogical:
	case OpShiftRightArithmetic:
	{
		auto &out_type = get<SPIRType>(cop.basetype);
		auto &type0 = expression_type(cop.arguments[0]);
		SPIRType::BaseType input_type = cop.opcode == OpShiftRightLogical    ? SPIRType::UInt :
		                                cop.opcode == OpShiftRightArithmetic ? SPIRType::Int :
		                                                                       type0.basetype;
		string lhs = to_enclosed_expression(cop.arguments[0]);
		if (type0.basetype != input_type)
		{
			auto expected = type0;
			expected.basetype = input_type;
			lhs = bitcast_expression(expected, type0.basetype, lhs);
		}
		string rhs = to_unsigned_expression(cop.arguments[1], type0.vecsize);
		if (needs_enclose_expression(rhs))
			rhs = enclose_expression(rhs);
		string expr = join(lhs, cop.opcode == OpShiftLeftLogical ? " << " : " >> ", rhs);
		if (out_type.basetype != input_type)
			expr = bitcast_expression(out_type, input_type, expr);
		return expr;
	}

	default:
		return CompilerGLSL::constant_op_expression(cop);
	}
}

bool CompilerWGSL::emit_complex_bitcast(uint32_t, uint32_t, uint32_t)
{
	// WGSL bitcast<T>() handles all bitcasts between types of the same size, e.g. u32 <-> vec2<f16>.
	return false;
}

void CompilerWGSL::emit_instruction(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);
	uint32_t length = instruction.length;

	uint32_t integer_width = get_integer_width_for_instruction(instruction);
	auto int_type = to_signed_basetype(integer_width);
	auto uint_type = to_unsigned_basetype(integer_width);

	switch (opcode)
	{
	case OpLoad:
	{
		if (is_atomic_pointer(ops[2]))
		{
			uint32_t result_type = ops[0];
			uint32_t id = ops[1];
			forced_temporaries.insert(id);
			emit_op(result_type, id, join("atomicLoad(&", to_expression(ops[2]), ")"), false);
			break;
		}
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	case OpStore:
	{
		if (is_atomic_pointer(ops[0]))
		{
			statement("atomicStore(&", to_expression(ops[0]), ", ", to_unpacked_expression(ops[1]), ");");
			register_write(ops[0]);
			break;
		}
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	case OpIEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "==", int_type, false, true);
		break;
	case OpINotEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "!=", int_type, false, true);
		break;
	case OpSLessThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<", int_type, false);
		break;
	case OpSLessThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<=", int_type, false);
		break;
	case OpSGreaterThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">", int_type, false);
		break;
	case OpSGreaterThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">=", int_type, false);
		break;
	case OpULessThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<", uint_type, false);
		break;
	case OpULessThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<=", uint_type, false);
		break;
	case OpUGreaterThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">", uint_type, false);
		break;
	case OpUGreaterThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">=", uint_type, false);
		break;

	case OpLogicalEqual:
	case OpFOrdEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "==", SPIRType::Unknown, false);
		break;
	case OpLogicalNotEqual:
	case OpFOrdNotEqual:
	case OpFUnordNotEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "!=", SPIRType::Unknown, false);
		break;
	case OpFOrdLessThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<", SPIRType::Unknown, false);
		break;
	case OpFOrdLessThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<=", SPIRType::Unknown, false);
		break;
	case OpFOrdGreaterThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">", SPIRType::Unknown, false);
		break;
	case OpFOrdGreaterThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">=", SPIRType::Unknown, false);
		break;
	case OpFUnordEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "!=", SPIRType::Unknown, true);
		break;
	case OpFUnordLessThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">=", SPIRType::Unknown, true);
		break;
	case OpFUnordLessThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], ">", SPIRType::Unknown, true);
		break;
	case OpFUnordGreaterThan:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<=", SPIRType::Unknown, true);
		break;
	case OpFUnordGreaterThanEqual:
		emit_vector_compare(ops[0], ops[1], ops[2], ops[3], "<", SPIRType::Unknown, true);
		break;

	case OpLogicalNot:
		emit_unary_op(ops[0], ops[1], ops[2], "!");
		break;

	case OpLogicalAnd:
		if (get<SPIRType>(ops[0]).vecsize > 1)
			emit_binary_op(ops[0], ops[1], ops[2], ops[3], "&");
		else
			emit_binary_op(ops[0], ops[1], ops[2], ops[3], "&&");
		break;

	case OpLogicalOr:
		if (get<SPIRType>(ops[0]).vecsize > 1)
			emit_binary_op(ops[0], ops[1], ops[2], ops[3], "|");
		else
			emit_binary_op(ops[0], ops[1], ops[2], ops[3], "||");
		break;

	case OpShiftLeftLogical:
		emit_shift_op(ops[0], ops[1], ops[2], ops[3], "<<", expression_type(ops[2]).basetype);
		break;
	case OpShiftRightLogical:
		emit_shift_op(ops[0], ops[1], ops[2], ops[3], ">>", uint_type);
		break;
	case OpShiftRightArithmetic:
		emit_shift_op(ops[0], ops[1], ops[2], ops[3], ">>", int_type);
		break;

	case OpFMod:
	{
		uint32_t op0 = ops[2];
		uint32_t op1 = ops[3];
		auto a = to_enclosed_unpacked_expression(op0);
		auto b = to_enclosed_unpacked_expression(op1);
		emit_op(ops[0], ops[1], join(a, " - ", b, " * floor(", a, " / ", b, ")"),
		        should_forward(op0) && should_forward(op1));
		inherit_expression_dependencies(ops[1], op0);
		inherit_expression_dependencies(ops[1], op1);
		break;
	}

	case OpFRem:
		emit_binary_op(ops[0], ops[1], ops[2], ops[3], "%");
		break;

	case OpIsNan:
		emit_isnan_isinf(ops[0], ops[1], ops[2], true);
		break;
	case OpIsInf:
		emit_isnan_isinf(ops[0], ops[1], ops[2], false);
		break;

	case OpSelect:
		emit_select_op(ops[0], ops[1], ops[2], ops[3], ops[4]);
		break;

	case OpUConvert:
	case OpSConvert:
	{
		auto &out_type = get<SPIRType>(ops[0]);
		if (lower_transient_16bit_integers && out_type.width == 16)
		{
			// Truncate to 16 bits (sign-extending for SConvert), but keep the value in a 32-bit integer.
			auto &in_type = expression_type(ops[2]);
			auto wide_type = out_type;
			wide_type.width = 32;
			string expr;
			if (opcode == OpUConvert)
			{
				wide_type.basetype = SPIRType::UInt;
				string value = to_enclosed_unpacked_expression(ops[2]);
				if (in_type.basetype != SPIRType::UInt)
					value = bitcast_expression(wide_type, in_type.basetype, value);
				expr = join(value, " & ", splat(wide_type, "0xffffu"));
			}
			else
			{
				wide_type.basetype = SPIRType::Int;
				string value = to_enclosed_unpacked_expression(ops[2]);
				if (in_type.basetype != SPIRType::Int)
					value = bitcast_expression(wide_type, in_type.basetype, value);
				auto shift_type = wide_type;
				shift_type.basetype = SPIRType::UInt;
				expr = join("(", value, " << ", splat(shift_type, "16u"), ") >> ", splat(shift_type, "16u"));
			}
			emit_op(ops[0], ops[1], expr, should_forward(ops[2]));
			inherit_expression_dependencies(ops[1], ops[2]);
			break;
		}
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	case OpBeginInvocationInterlockEXT:
	case OpEndInvocationInterlockEXT:
		SPIRV_CROSS_THROW("Fragment shader interlock is not supported in WGSL.");

	case OpSDot:
	case OpUDot:
	case OpSUDot:
	case OpSDotAccSat:
	case OpUDotAccSat:
	case OpSUDotAccSat:
		SPIRV_CROSS_THROW("Integer dot products are not supported in WGSL.");

	case OpOuterProduct:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t a = ops[2];
		uint32_t b = ops[3];
		auto &type = get<SPIRType>(result_type);
		string expr = join(type_to_glsl(type), "(");
		for (uint32_t i = 0; i < type.columns; i++)
		{
			expr += join(to_enclosed_unpacked_expression(a), " * ", to_extract_component_expression(b, i));
			if (i + 1 < type.columns)
				expr += ", ";
		}
		expr += ")";
		emit_op(result_type, id, expr, should_forward(a) && should_forward(b));
		inherit_expression_dependencies(id, a);
		inherit_expression_dependencies(id, b);
		break;
	}

	case OpBitFieldInsert:
	case OpBitFieldSExtract:
	case OpBitFieldUExtract:
	case OpBitCount:
	case OpBitReverse:
		emit_bitfield_op(instruction);
		break;

	case OpDPdx:
		emit_unary_func_op(ops[0], ops[1], ops[2], "dpdx");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpDPdy:
		emit_unary_func_op(ops[0], ops[1], ops[2], "dpdy");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpFwidth:
		emit_unary_func_op(ops[0], ops[1], ops[2], "fwidth");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpDPdxFine:
		emit_unary_func_op(ops[0], ops[1], ops[2], "dpdxFine");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpDPdyFine:
		emit_unary_func_op(ops[0], ops[1], ops[2], "dpdyFine");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpFwidthFine:
		emit_unary_func_op(ops[0], ops[1], ops[2], "fwidthFine");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpDPdxCoarse:
		emit_unary_func_op(ops[0], ops[1], ops[2], "dpdxCoarse");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpDPdyCoarse:
		emit_unary_func_op(ops[0], ops[1], ops[2], "dpdyCoarse");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;
	case OpFwidthCoarse:
		emit_unary_func_op(ops[0], ops[1], ops[2], "fwidthCoarse");
		register_control_dependent_expression(ops[1]);
		require_enable(uses_implicit_derivatives);
		break;

	case OpQuantizeToF16:
		emit_unary_func_op(ops[0], ops[1], ops[2], "quantizeToF16");
		break;

	case OpArrayLength:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		auto e =
		    access_chain_internal(ops[2], &ops[3], length - 3, ACCESS_CHAIN_INDEX_IS_LITERAL_BIT, nullptr, nullptr);
		string expr = join("arrayLength(&", e, ")");
		if (get<SPIRType>(result_type).basetype != SPIRType::UInt)
			expr = join(type_to_glsl(get<SPIRType>(result_type)), "(", expr, ")");
		set<SPIRExpression>(id, expr, result_type, true);
		break;
	}

	case OpControlBarrier:
	case OpMemoryBarrier:
	{
		uint32_t execution_scope = 0;
		uint32_t semantics;
		if (opcode == OpMemoryBarrier)
			semantics = evaluate_constant_u32(ops[1]);
		else
		{
			execution_scope = evaluate_constant_u32(ops[0]);
			semantics = evaluate_constant_u32(ops[2]);
		}

		semantics = mask_relevant_memory_semantics(semantics);

		if (opcode == OpMemoryBarrier)
		{
			// The following control barrier covers this memory barrier.
			const Instruction *next = get_next_instruction_in_block(instruction);
			if (next && next->op == OpControlBarrier)
				break;
		}

		if (semantics || opcode == OpControlBarrier)
		{
			assert(current_emitting_block);
			flush_control_dependent_expressions(current_emitting_block->self);
			flush_all_active_variables();
		}

		if (semantics & MemorySemanticsUniformMemoryMask)
			statement("storageBarrier();");
		if (semantics & MemorySemanticsImageMemoryMask)
			statement("textureBarrier();");
		if ((opcode == OpControlBarrier && execution_scope == ScopeWorkgroup) ||
		    (semantics & MemorySemanticsWorkgroupMemoryMask))
			statement("workgroupBarrier();");
		break;
	}

	case OpAtomicFAddEXT:
	case OpAtomicFMinEXT:
	case OpAtomicFMaxEXT:
		SPIRV_CROSS_THROW("Floating point atomics are not supported in WGSL.");

	case OpAtomicLoad:
	case OpAtomicStore:
	case OpAtomicExchange:
	case OpAtomicCompareExchange:
	case OpAtomicCompareExchangeWeak:
	case OpAtomicIIncrement:
	case OpAtomicIDecrement:
	case OpAtomicIAdd:
	case OpAtomicISub:
	case OpAtomicSMin:
	case OpAtomicUMin:
	case OpAtomicSMax:
	case OpAtomicUMax:
	case OpAtomicAnd:
	case OpAtomicOr:
	case OpAtomicXor:
		emit_atomic_op(instruction);
		break;

	case OpImage:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		auto &e = emit_op(result_type, id, to_image_expression(ops[2]), true, true);
		auto *var = maybe_get_backing_variable(ops[2]);
		e.loaded_from = var ? var->self : ID(0);
		break;
	}

	case OpImageRead:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t img = ops[2];
		uint32_t coord = ops[3];
		auto &type = expression_type(img);
		auto &coord_type = expression_type(coord);
		uint32_t dims = type.image.dim == Dim1D ? 1 : (type.image.dim == Dim3D ? 3 : 2);
		string coord_expr = to_enclosed_unpacked_expression(coord);
		string expr = join("textureLoad(", to_image_expression(img), ", ",
		                   component_expression(coord_expr, coord_type.vecsize, 0, dims));
		if (type.image.arrayed)
			expr += join(", ", component_expression(coord_expr, coord_type.vecsize, dims, 1));
		if (type.image.sampled != 2)
			expr += ", 0";
		expr += ")";
		auto &out_type = get<SPIRType>(result_type);
		if (out_type.vecsize < 4)
			expr = join(expr, ".", string(swizzle_components, out_type.vecsize));
		emit_op(result_type, id, expr, false);
		inherit_expression_dependencies(id, coord);
		break;
	}

	case OpImageWrite:
	{
		uint32_t img = ops[0];
		uint32_t coord = ops[1];
		uint32_t value = ops[2];
		auto &type = expression_type(img);
		auto &coord_type = expression_type(coord);
		auto &value_type = expression_type(value);
		uint32_t dims = type.image.dim == Dim1D ? 1 : (type.image.dim == Dim3D ? 3 : 2);
		string coord_expr = to_enclosed_unpacked_expression(coord);
		string expr = join("textureStore(", to_image_expression(img), ", ",
		                   component_expression(coord_expr, coord_type.vecsize, 0, dims));
		if (type.image.arrayed)
			expr += join(", ", component_expression(coord_expr, coord_type.vecsize, dims, 1));

		string value_expr = to_unpacked_expression(value);
		if (value_type.vecsize < 4)
		{
			auto vec4_type = value_type;
			vec4_type.vecsize = 4;
			string zero = value_type.basetype == SPIRType::Float ? "0.0f" : "0";
			string one = value_type.basetype == SPIRType::Float ? "1.0f" : "1";
			value_expr = join(type_to_glsl(vec4_type), "(", value_expr);
			for (uint32_t i = value_type.vecsize; i < 3; i++)
				value_expr += join(", ", zero);
			value_expr += join(", ", one, ")");
		}
		expr += join(", ", value_expr, ");");
		statement(expr);
		register_write(img);
		break;
	}

	case OpImageQuerySizeLod:
	case OpImageQuerySize:
	case OpImageQueryLevels:
	case OpImageQuerySamples:
		emit_image_query(instruction);
		break;

	case OpImageQueryLod:
		SPIRV_CROSS_THROW("textureQueryLod is not supported in WGSL.");

	case OpImageTexelPointer:
		SPIRV_CROSS_THROW("Image atomics are not supported in WGSL.");

	case OpUMulExtended:
	case OpSMulExtended:
	case OpIAddCarry:
	case OpISubBorrow:
		SPIRV_CROSS_THROW("Extended arithmetic is not supported in WGSL.");

	case OpExtInst:
	{
		auto &ext = get<SPIRExtension>(ops[2]).ext;
		if (ext == SPIRExtension::NonSemanticDebugPrintf)
			SPIRV_CROSS_THROW("Debug printf is not supported in WGSL.");
		if (ext == SPIRExtension::SPV_AMD_shader_ballot || ext == SPIRExtension::SPV_AMD_shader_trinary_minmax ||
		    ext == SPIRExtension::SPV_AMD_shader_explicit_vertex_parameter || ext == SPIRExtension::SPV_AMD_gcn_shader)
			SPIRV_CROSS_THROW("AMD extended instructions are not supported in WGSL.");
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	case OpSubgroupAllKHR:
	case OpSubgroupAnyKHR:
	case OpSubgroupAllEqualKHR:
	case OpSubgroupBallotKHR:
	case OpSubgroupFirstInvocationKHR:
	case OpSubgroupReadInvocationKHR:
		SPIRV_CROSS_THROW("KHR subgroup instructions are not supported in WGSL.");

	case OpEmitVertex:
	case OpEndPrimitive:
	case OpEmitStreamVertex:
	case OpEndStreamPrimitive:
		SPIRV_CROSS_THROW("Geometry shaders are not supported in WGSL.");

	case OpReadClockKHR:
		SPIRV_CROSS_THROW("Shader clocks are not supported in WGSL.");

	case OpIsHelperInvocationEXT:
		SPIRV_CROSS_THROW("Helper invocation queries are not supported in WGSL.");

	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
}

void CompilerWGSL::emit_glsl_op(uint32_t result_type, uint32_t id, uint32_t eop, const uint32_t *args, uint32_t count)
{
	auto op = static_cast<GLSLstd450>(eop);
	auto int_type = to_signed_basetype(32);
	auto uint_type = to_unsigned_basetype(32);

	switch (op)
	{
	case GLSLstd450RoundEven:
		emit_unary_func_op(result_type, id, args[0], "round");
		break;

	case GLSLstd450InverseSqrt:
		emit_unary_func_op(result_type, id, args[0], "inverseSqrt");
		break;

	case GLSLstd450Atan2:
		emit_binary_func_op(result_type, id, args[0], args[1], "atan2");
		break;

	case GLSLstd450Normalize:
		// WGSL normalize() only accepts vectors.
		if (get<SPIRType>(result_type).vecsize == 1)
			emit_unary_func_op(result_type, id, args[0], "sign");
		else
			emit_unary_func_op(result_type, id, args[0], "normalize");
		break;

	case GLSLstd450FaceForward:
	case GLSLstd450Reflect:
	case GLSLstd450Refract:
	{
		const char *func =
		    op == GLSLstd450FaceForward ? "faceForward" : (op == GLSLstd450Reflect ? "reflect" : "refract");
		auto &type = get<SPIRType>(result_type);
		if (type.vecsize == 1)
		{
			// WGSL only accepts vectors, so compute the result in a 2-component vector.
			auto vec2_type = type;
			vec2_type.vecsize = 2;
			auto vec2_name = type_to_glsl(vec2_type);
			string zero = type.basetype == SPIRType::Half ? "0.0h" : "0.0f";
			auto widen = [&](uint32_t arg)
			{ return join(vec2_name, "(", to_unpacked_expression(arg), ", ", zero, ")"); };

			string expr = join(func, "(", widen(args[0]), ", ", widen(args[1]), ", ");
			if (op == GLSLstd450Reflect)
				expr = join(func, "(", widen(args[0]), ", ", widen(args[1]), ").x");
			else if (op == GLSLstd450Refract)
				expr += join(to_unpacked_expression(args[2]), ").x");
			else
				expr += join(widen(args[2]), ").x");

			bool forward = should_forward(args[0]) && should_forward(args[1]) &&
			               (op == GLSLstd450Reflect || should_forward(args[2]));
			emit_op(result_type, id, expr, forward);
			inherit_expression_dependencies(id, args[0]);
			inherit_expression_dependencies(id, args[1]);
			if (op != GLSLstd450Reflect)
				inherit_expression_dependencies(id, args[2]);
		}
		else if (op == GLSLstd450Reflect)
			emit_binary_func_op(result_type, id, args[0], args[1], func);
		else
			emit_trinary_func_op(result_type, id, args[0], args[1], args[2], func);
		break;
	}

	case GLSLstd450NMin:
		emit_binary_func_op(result_type, id, args[0], args[1], "min");
		break;
	case GLSLstd450NMax:
		emit_binary_func_op(result_type, id, args[0], args[1], "max");
		break;
	case GLSLstd450NClamp:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "clamp");
		break;

	case GLSLstd450PackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "pack4x8snorm");
		break;
	case GLSLstd450PackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "pack4x8unorm");
		break;
	case GLSLstd450PackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "pack2x16snorm");
		break;
	case GLSLstd450PackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "pack2x16unorm");
		break;
	case GLSLstd450PackHalf2x16:
		emit_unary_func_op(result_type, id, args[0], "pack2x16float");
		break;
	case GLSLstd450UnpackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpack4x8snorm");
		break;
	case GLSLstd450UnpackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpack4x8unorm");
		break;
	case GLSLstd450UnpackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack2x16snorm");
		break;
	case GLSLstd450UnpackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack2x16unorm");
		break;
	case GLSLstd450UnpackHalf2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack2x16float");
		break;

	case GLSLstd450FindILsb:
	{
		auto basetype = expression_type(args[0]).basetype;
		emit_unary_func_op_cast(result_type, id, args[0], "firstTrailingBit", basetype, basetype);
		break;
	}
	case GLSLstd450FindSMsb:
		emit_unary_func_op_cast(result_type, id, args[0], "firstLeadingBit", int_type, int_type);
		break;
	case GLSLstd450FindUMsb:
		emit_unary_func_op_cast(result_type, id, args[0], "firstLeadingBit", uint_type, uint_type);
		break;

	case GLSLstd450Modf:
	case GLSLstd450Frexp:
	{
		bool is_modf = op == GLSLstd450Modf;
		auto tmp = join("_", id, is_modf ? "_modf" : "_frexp");
		statement("let ", tmp, " = ", is_modf ? "modf(" : "frexp(", to_unpacked_expression(args[0]), ");");
		statement(to_expression(args[1]), " = ", tmp, is_modf ? ".whole;" : ".exp;");
		register_write(args[1]);
		emit_op(result_type, id, join(tmp, ".fract"), false);
		break;
	}

	case GLSLstd450ModfStruct:
	case GLSLstd450FrexpStruct:
	{
		bool is_modf = op == GLSLstd450ModfStruct;
		auto tmp = join("_", id, is_modf ? "_modf" : "_frexp");
		statement("let ", tmp, " = ", is_modf ? "modf(" : "frexp(", to_unpacked_expression(args[0]), ");");
		emit_op(
		    result_type, id,
		    join(type_to_glsl(get<SPIRType>(result_type)), "(", tmp, ".fract, ", tmp, is_modf ? ".whole)" : ".exp)"),
		    false);
		break;
	}

	case GLSLstd450MatrixInverse:
	{
		auto &type = get<SPIRType>(result_type);
		if (type.basetype != SPIRType::Float || type.vecsize != type.columns)
			SPIRV_CROSS_THROW("Unsupported matrix inverse.");

		const char *func = nullptr;
		bool *flag = nullptr;
		switch (type.vecsize)
		{
		case 2:
			func = "spvInverse2x2";
			flag = &requires_inverse_2x2;
			break;
		case 3:
			func = "spvInverse3x3";
			flag = &requires_inverse_3x3;
			break;
		case 4:
			func = "spvInverse4x4";
			flag = &requires_inverse_4x4;
			break;
		default:
			SPIRV_CROSS_THROW("Unsupported matrix inverse.");
		}

		if (!*flag)
		{
			*flag = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, id, args[0], func);
		break;
	}

	case GLSLstd450PackDouble2x32:
	case GLSLstd450UnpackDouble2x32:
		SPIRV_CROSS_THROW("64-bit types are not supported in WGSL.");

	case GLSLstd450InterpolateAtCentroid:
	case GLSLstd450InterpolateAtSample:
	case GLSLstd450InterpolateAtOffset:
		SPIRV_CROSS_THROW("Interpolation functions are not supported in WGSL.");

	default:
		CompilerGLSL::emit_glsl_op(result_type, id, eop, args, count);
		break;
	}
}
