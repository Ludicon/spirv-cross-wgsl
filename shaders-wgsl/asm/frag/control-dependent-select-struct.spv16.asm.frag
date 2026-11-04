; SPIR-V
; Version: 1.6
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 50
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %Samp %vUV %FragColor %gl_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %S "S"
               OpMemberName %S 0 "v"
               OpName %Samp "Samp"
               OpName %vUV "vUV"
               OpName %FragColor "FragColor"
               OpName %gl_FragCoord "gl_FragCoord"
               OpDecorate %Samp Binding 0
               OpDecorate %Samp DescriptorSet 0
               OpDecorate %vUV Location 0
               OpDecorate %FragColor Location 0
               OpDecorate %gl_FragCoord BuiltIn FragCoord
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %S = OpTypeStruct %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %Samp = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
  %float_100 = OpConstant %float 100
       %bool = OpTypeBool
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %Samp
         %18 = OpLoad %v2float %vUV
         %19 = OpImageSampleImplicitLod %v4float %14 %18
         %20 = OpVectorShuffle %v2float %18 %18 1 0
         %23 = OpImageSampleImplicitLod %v4float %14 %20
         %24 = OpCompositeConstruct %S %19
         %25 = OpCompositeConstruct %S %23
         %31 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %32 = OpLoad %float %31
         %35 = OpFOrdLessThan %bool %32 %float_100
         %38 = OpSelect %S %35 %24 %25
         %39 = OpCompositeExtract %v4float %38 0
               OpStore %FragColor %39
               OpReturn
               OpFunctionEnd
