@group(0) @binding(0) var uShadow2DArray : texture_depth_2d_array;
@group(0) @binding(16) var uShadow2DArray_sampler : sampler_comparison;

var<private> FragColor : vec4f;
var<private> vUV : vec4f;
var<private> vLod : f32;

fn frag_main()
{
    FragColor = vec4f(textureSampleCompareLevel(uShadow2DArray, uShadow2DArray_sampler, vUV.xy, i32(round(vUV.z)), vUV.w));
}

struct SPIRV_Cross_Input
{
    @location(0) vUV : vec4f,
    @location(1) vLod : f32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vUV = stage_input.vUV;
    vLod = stage_input.vLod;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
