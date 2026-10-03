@group(0) @binding(0) var uT : texture_depth_2d;
@group(0) @binding(16) var uT_sampler : sampler_comparison;

var<private> FragColor : vec4f;
var<private> vUV : vec3f;

fn frag_main()
{
    FragColor = textureGatherCompare(uT, uT_sampler, vUV.xy, vUV.z);
}

struct SPIRV_Cross_Input
{
    @location(0) vUV : vec3f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vUV = stage_input.vUV;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
