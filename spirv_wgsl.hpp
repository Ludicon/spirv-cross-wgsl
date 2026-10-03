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
// binding + combined_sampler_binding_offset in the same group.
//
// WGSL has no push constants. Push constant blocks are emitted as uniform buffers using
// push_constant_group and push_constant_binding.
class CompilerWGSL : public CompilerGLSL
{
public:
	struct Options
	{
		// Binding offset applied to the sampler part of combined image samplers.
		uint32_t combined_sampler_binding_offset = 16;

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
	void emit_subgroup_op(const Instruction &i) override;
	bool skip_argument(uint32_t id) const override;
	using CompilerGLSL::variable_decl;

private:
	Options wgsl_options;

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

	// Pointer parameters which need to be dereferenced when used as expressions.
	std::unordered_set<uint32_t> pointer_parameters;

	// Struct members and variables accessed with atomic operations.
	std::unordered_set<uint64_t> atomic_members;
	std::unordered_set<uint32_t> atomic_variables;

	bool requires_inverse_2x2 = false;
	bool requires_inverse_3x3 = false;
	bool requires_inverse_4x4 = false;
	bool requires_f16 = false;
	bool requires_subgroups = false;
	bool requires_clip_distances = false;
	bool requires_dual_source_blending = false;
	bool uses_implicit_derivatives = false;

	uint32_t clip_distance_count = 0;
};
} // namespace SPIRV_CROSS_NAMESPACE

#endif
