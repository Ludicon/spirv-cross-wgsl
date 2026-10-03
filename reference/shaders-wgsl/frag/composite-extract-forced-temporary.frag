diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var Texture : texture_2d<f32>;
@group(0) @binding(16) var Texture_sampler : sampler;

var<private> vTexCoord : vec2f;
var<private> FragColor : vec4f;

fn frag_main()
{
    var f : f32 = textureSample(Texture, Texture_sampler, vTexCoord).x;
    FragColor = vec4f(f * f);
}

struct SPIRV_Cross_Input
{
    @location(0) vTexCoord : vec2f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vTexCoord = stage_input.vTexCoord;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
