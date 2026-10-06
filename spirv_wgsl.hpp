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

#ifndef SPIRV_WGSL_HPP
#define SPIRV_WGSL_HPP

#include "spirv_glsl.hpp"
#include <unordered_set>

namespace SPIRV_CROSS_NAMESPACE
{
using namespace SPIRV_CROSS_SPV_HEADER_NAMESPACE;

// Emits WGSL (WebGPU Shading Language) from SPIR-V.
//
// Stage inputs and outputs are declared as module scope private variables, and a wrapper
// entry point copies them from and to the WGSL entry point parameters and return value,
// similar to how the HLSL backend works.
//
// WGSL has no combined image samplers. Combined image samplers are split into a texture which keeps
// the original binding, and a sampler named <texture>_sampler, which is assigned
// binding + combined_sampler_binding_offset in the same group. Alternatively, resolve_binding_conflicts
// assigns bindings the same way as tint's SPIR-V reader.
//
// WGSL has no push constants. Push constant blocks are emitted as uniform buffers using
// push_constant_group and push_constant_binding.

// Warnings for constructs which compile, but whose translation is not exact.
// Retrieve them with CompilerWGSL::get_warnings() after compile().
enum WGSLWarning
{
	// Infinity or NaN constants are computed at runtime with spvNonFinite(), since WGSL rejects
	// non-finite values in constant expressions.
	WGSL_WARNING_NON_FINITE_CONSTANT = 0,

	// Depth comparison with an explicit non-zero LOD or gradients. WGSL only supports comparisons at LOD 0.
	WGSL_WARNING_DEPTH_COMPARE_LOD,

	// LOD bias outside of fragment shaders is ignored.
	WGSL_WARNING_IGNORED_BIAS,

	// Strong compare-exchange atomics are emitted as atomicCompareExchangeWeak(), which may fail spuriously.
	WGSL_WARNING_WEAK_COMPARE_EXCHANGE,

	// Builtin outputs which do not exist in WGSL (e.g. gl_PointSize) are written but ignored.
	WGSL_WARNING_IGNORED_BUILTIN,

	WGSL_WARNING_COUNT
};

class CompilerWGSL : public CompilerGLSL
{
public:
	struct Warning
	{
		WGSLWarning kind;
		std::string message;
	};

	struct Options
	{
		// Binding offset applied to the sampler part of combined image samplers.
		uint32_t combined_sampler_binding_offset = 16;

		// If enabled, assign bindings the same way as tint's SPIR-V reader (the SPIRV-Tools
		// split-combined-image-sampler and resolve-binding-conflicts passes): the sampler part of a combined
		// image sampler shares the binding of its texture, and then, within each group, the resources used by
		// the entry point are sorted by binding (samplers after textures) and any binding which is not greater
		// than the previous one is moved to the previous binding + 1. combined_sampler_binding_offset is ignored.
		// In library modules there is no entry point, so every resource is considered used.
		bool resolve_binding_conflicts = false;

		// Group and binding used for push constant blocks.
		uint32_t push_constant_group = 0;
		uint32_t push_constant_binding = 0;

		// SPIR-V allows implicit derivatives (textureSample(), dpdx(), etc.) in non-uniform control flow,
		// but WGSL rejects them by default. If enabled, shaders using implicit derivatives
		// emit "diagnostic(off, derivative_uniformity);".
		bool allow_non_uniform_derivatives = true;
	};

	explicit CompilerWGSL(std::vector<uint32_t> spirv_)
	    : CompilerGLSL(std::move(spirv_))
	{
	}

	CompilerWGSL(const uint32_t *ir_, size_t size)
	    : CompilerGLSL(ir_, size)
	{
	}

	explicit CompilerWGSL(const ParsedIR &ir_)
	    : CompilerGLSL(ir_)
	{
	}

	explicit CompilerWGSL(ParsedIR &&ir_)
	    : CompilerGLSL(std::move(ir_))
	{
	}

	const Options &get_wgsl_options() const
	{
		return wgsl_options;
	}

	void set_wgsl_options(const Options &opts)
	{
		wgsl_options = opts;
	}

	std::string compile() override;

	// Warnings produced by the last call to compile(). Disabled warnings are not reported.
	const SmallVector<Warning> &get_warnings() const
	{
		return warnings;
	}

	void set_warning_enabled(WGSLWarning kind, bool enabled);
	bool is_warning_enabled(WGSLWarning kind) const;

	// Short names, e.g. "non-finite-constant", used for diagnostics and command line options.
	static const char *get_warning_name(WGSLWarning kind);
	static bool get_warning_from_name(const std::string &name, WGSLWarning &kind);

protected:
	std::string type_to_glsl(const SPIRType &type, uint32_t id = 0) override;
	std::string type_to_array_glsl(const SPIRType &type, uint32_t variable_id) override;
	std::string image_type_glsl(const SPIRType &type, uint32_t id = 0, bool member = false) override;
	std::string variable_decl(const SPIRType &type, const std::string &name, uint32_t id = 0) override;
	std::string variable_decl(const SPIRVariable &variable) override;
	std::string to_name(uint32_t id, bool allow_alias = true) const override;
	std::string builtin_to_glsl(BuiltIn builtin, StorageClass storage) override;
	std::string bitcast_glsl_op(const SPIRType &result_type, const SPIRType &argument_type) override;
	std::string to_ternary_expression(const SPIRType &result_type, uint32_t select, uint32_t true_value,
	                                  uint32_t false_value) override;
	std::string to_function_name(const TextureFunctionNameArguments &args) override;
	std::string to_function_args(const TextureFunctionArguments &args, bool *p_forward) override;
	std::string to_func_call_arg(const SPIRFunction::Parameter &arg, uint32_t id) override;
	std::string to_initializer_expression(const SPIRVariable &var) override;
	std::string constant_op_expression(const SPIRConstantOp &cop) override;
	std::string to_qualifiers_glsl(uint32_t id) override;

	void emit_header() override;
	void emit_function_prototype(SPIRFunction &func, const Bitset &return_flags) override;
	void emit_instruction(const Instruction &instruction) override;
	void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t *args,
	                  uint32_t count) override;
	void emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id) override;
	void emit_buffer_block(const SPIRVariable &var) override;
	void emit_push_constant_block(const SPIRVariable &var) override;
	void emit_uniform(const SPIRVariable &var) override;
	void replace_illegal_names() override;
	void emit_block_hints(const SPIRBlock &block) override;
	bool emit_complex_bitcast(uint32_t result_type, uint32_t id, uint32_t op0) override;
	bool member_is_non_native_row_major_matrix(const SPIRType &type, uint32_t index,
	                                           bool is_layout_disabled = false) override;
	std::string unpack_expression_type(std::string expr_str, const SPIRType &type, uint32_t physical_type_id,
	                                   bool packed, bool row_major) override;
	std::string convert_row_major_matrix(std::string exp_str, const SPIRType &exp_type, uint32_t physical_type_id,
	                                     bool is_packed, bool relaxed) override;
	void emit_store_statement(uint32_t lhs_expression, uint32_t rhs_expression) override;
	std::string declare_temporary(uint32_t result_type, uint32_t result_id) override;
	std::string convert_half_to_string(const SPIRConstant &value, uint32_t col, uint32_t row) override;
	std::string convert_float_to_string(const SPIRConstant &value, uint32_t col, uint32_t row) override;
	std::string constant_expression_vector(const SPIRConstant &c, uint32_t vector) override;
	std::string non_finite_float_expression(uint32_t bits);
	void emit_subgroup_op(const Instruction &i) override;
	bool skip_argument(uint32_t id) const override;
	using CompilerGLSL::variable_decl;

private:
	Options wgsl_options;

	SmallVector<Warning> warnings;
	uint32_t disabled_warnings = 0;
	void warn(WGSLWarning kind, const std::string &message);
	std::string warning_location();

	struct StageIOMember
	{
		std::string lhs; // Expression of the private variable (or a sub-element of it).
		std::string name; // Member name in the entry point I/O struct.
		uint32_t type_id = 0; // SPIR-V type of the private variable sub-element.
		std::string attributes; // @location, @builtin, @interpolate, etc.
		std::string wgsl_type; // Type of the I/O struct member.
		uint32_t sort_key = 0;
	};

	void emit_resources();
	void resolve_binding_conflicts();
	std::string binding_attributes(uint32_t var_id, bool sampler_part = false);
	void emit_struct_wgsl(SPIRType &type);
	void prepare_buffer_layouts();
	uint32_t build_padded_physical_type(uint32_t type_id, bool transpose);
	uint32_t struct_padded_size(uint32_t struct_type_id);
	void wgsl_layout(uint32_t type_id, bool transpose, uint32_t &align, uint32_t &size);
	std::string declared_type_to_wgsl(uint32_t type_id, bool transpose);
	void emit_constants_and_structs();
	void emit_entry_point_wrapper();
	void emit_helper_functions();
	void collect_stage_io(StorageClass storage, SmallVector<StageIOMember> &members);
	void add_stage_io_member(SmallVector<StageIOMember> &members, const std::string &lhs, const std::string &name,
	                         uint32_t type_id, uint32_t &location, const Bitset &flags, StorageClass storage,
	                         bool flat_required);
	void add_stage_io_builtin(SmallVector<StageIOMember> &members, BuiltIn builtin, uint32_t type_id,
	                          StorageClass storage, bool invariant);
	std::string interpolation_attributes(const Bitset &flags, const SPIRType &type, StorageClass storage,
	                                     bool flat_required);
	std::string wgsl_builtin_name(BuiltIn builtin, StorageClass storage) const;
	std::string wgsl_builtin_type(BuiltIn builtin) const;

	std::string get_inner_entry_point_name() const;
	std::string scalar_type_name(const SPIRType &type) const;
	static std::string base_type_name(const SPIRType &type);
	std::string address_space(StorageClass storage) const;
	std::string ptr_type(const SPIRType &pointee, StorageClass storage, uint32_t id);
	std::string to_sampler_expression(uint32_t id);
	std::string to_image_expression(uint32_t id);
	std::string to_unsigned_expression(uint32_t id, uint32_t vecsize);
	std::string splat(const SPIRType &type, const std::string &scalar_literal);
	bool is_pointer_parameter(const SPIRFunction::Parameter &arg) const;
	bool is_depth_texture(uint32_t id);
	bool is_comparison_sampler(uint32_t id) const;
	bool is_atomic_pointer(uint32_t ptr_id);
	bool resolve_atomic_target(uint32_t ptr_id, uint32_t &type_id, uint32_t &member, uint32_t &var_id);
	bool member_is_atomic(uint32_t type_id, uint32_t index) const;
	bool variable_is_atomic(uint32_t var_id) const;
	void analyze_atomics();
	void analyze_mutable_temporaries();
	void analyze_single_store_variables();
	void analyze_transient_16bit_integers();
	bool is_16bit_integer_type(uint32_t type_id) const;
	void mark_atomic_pointer(uint32_t ptr_id);
	std::string wrap_atomic(const SPIRType &type, const std::string &base);
	std::string image_format_to_wgsl(ImageFormat fmt) const;
	std::string image_access_to_wgsl(const SPIRType &type, uint32_t id);

	void emit_image_query(const Instruction &instruction);
	void emit_atomic_op(const Instruction &instruction);
	void emit_shift_op(uint32_t result_type, uint32_t id, uint32_t op0, uint32_t op1, const char *op,
	                   SPIRType::BaseType input_type);
	void emit_vector_compare(uint32_t result_type, uint32_t id, uint32_t op0, uint32_t op1, const char *op,
	                         SPIRType::BaseType input_type, bool unordered, bool sign_invariant = false);
	void emit_isnan_isinf(uint32_t result_type, uint32_t id, uint32_t op0, bool is_nan);
	void emit_bitfield_op(const Instruction &instruction);
	void emit_select_op(uint32_t result_type, uint32_t id, uint32_t cond, uint32_t true_value, uint32_t false_value);
	void require_enable(bool &flag);
	std::string get_entry_point_wrapper_name() const;
	bool is_library_export(uint32_t func_id) const;
	bool implicit_lod_allowed() const;

	// Pointer parameters which need to be dereferenced when used as expressions.
	std::unordered_set<uint32_t> pointer_parameters;

	// Struct members and variables accessed with atomic operations.
	std::unordered_set<uint64_t> atomic_members;
	std::unordered_set<uint32_t> atomic_variables;

	// Temporaries which are modified after their declaration, and must be declared with var instead of let.
	std::unordered_set<uint32_t> mutable_temporaries;

	// Function local variables which are stored exactly once and never escape, which can be declared with let.
	// Variables passed to functions are only immutable if the callee parameter is passed by value, which is
	// resolved at emission time, since parameter write counts are computed while emitting the callee.
	std::unordered_set<uint32_t> single_store_variables;
	std::unordered_map<uint32_t, SmallVector<std::pair<uint32_t, uint32_t>>> variable_call_arguments;
	bool variable_can_be_let(uint32_t var_id) const;

	bool requires_inverse_2x2 = false;
	bool requires_inverse_3x3 = false;
	bool requires_inverse_4x4 = false;
	bool requires_f16 = false;
	bool requires_subgroups = false;
	bool requires_clip_distances = false;
	bool requires_dual_source_blending = false;
	bool uses_implicit_derivatives = false;
	bool requires_non_finite_helper = false;
	bool uses_workgroup_size_constant = false;

	// Bindings assigned by resolve_binding_conflicts(), for textures/buffers/samplers and for the sampler part
	// of combined image samplers.
	std::unordered_map<uint32_t, uint32_t> resolved_bindings;
	std::unordered_map<uint32_t, uint32_t> resolved_sampler_bindings;

	// WGSL has no 16-bit integers. If they are only used as truncated intermediates of int to float conversions,
	// they are carried in 32-bit integers.
	bool lower_transient_16bit_integers = false;

	uint32_t clip_distance_count = 0;
};
} // namespace SPIRV_CROSS_NAMESPACE

#endif
