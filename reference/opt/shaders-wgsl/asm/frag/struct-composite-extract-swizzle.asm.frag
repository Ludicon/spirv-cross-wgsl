diagnostic(off, derivative_uniformity);

struct Foo
{
    var1 : f32,
    var2 : f32,
}

var<private> _33 : Foo;

@group(0) @binding(0) var uSampler : texture_2d<f32>;
@group(0) @binding(16) var uSampler_sampler : sampler;

var<private> FragColor : vec4f;

fn frag_main()
{
    FragColor = textureSample(uSampler, uSampler_sampler, vec2f(_33.var1, _33.var2));
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
