diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var samp : texture_2d<f32>;
@group(0) @binding(16) var samp_sampler : sampler;

var<private> FragColor : vec4f;
var<private> vUV : vec2f;
var<private> vNormal : vec3f;

fn frag_main()
{
    FragColor = vec4f(textureSample(samp, samp_sampler, vUV).xyz, 1.0f);
    FragColor = vec4f(textureSample(samp, samp_sampler, vUV).xz, 1.0f, 4.0f);
    FragColor = vec4f(textureSample(samp, samp_sampler, vUV).xx, textureSample(samp, samp_sampler, (vUV + vec2f(0.100000001490116119384765625f))).yy);
    FragColor = vec4f(vNormal, 1.0f);
    FragColor = vec4f(vNormal + vec3f(1.7999999523162841796875f), 1.0f);
    FragColor = vec4f(vUV, vUV + vec2f(1.7999999523162841796875f));
}

struct SPIRV_Cross_Input
{
    @location(1) vNormal : vec3f,
    @location(2) vUV : vec2f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vNormal = stage_input.vNormal;
    vUV = stage_input.vUV;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
