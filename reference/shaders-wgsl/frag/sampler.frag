diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var uTex : texture_2d<f32>;
@group(0) @binding(16) var uTex_sampler : sampler;

var<private> FragColor : vec4f;
var<private> vColor : vec4f;
var<private> vTex : vec2f;

fn sample_texture(tex : texture_2d<f32>, tex_sampler : sampler, uv : vec2f) -> vec4f
{
    return textureSample(tex, tex_sampler, uv);
}

fn frag_main()
{
    let param = vTex;
    FragColor = vColor * sample_texture(uTex, uTex_sampler, param);
}

struct SPIRV_Cross_Input
{
    @location(0) vColor : vec4f,
    @location(1) vTex : vec2f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vColor = stage_input.vColor;
    vTex = stage_input.vTex;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
