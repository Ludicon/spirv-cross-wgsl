diagnostic(off, derivative_uniformity);

struct Count
{
    count : f32,
}

@group(0) @binding(1) var<uniform> _44 : Count;
@group(0) @binding(0) var tex : texture_2d<f32>;
@group(0) @binding(16) var tex_sampler : sampler;

var<private> vertex : vec4f;
var<private> fragColor : vec4f;

fn frag_main()
{
    var size : f32 = 1.0f / f32(vec2i(textureDimensions(tex, 0)).x);
    var r : f32 = 0.0f;
    var d : f32 = dpdx(vertex.x);
    for (var i : f32 = 0.0f; i < _44.count; i += 1.0f)
    {
        r += (size * d);
    }
    fragColor = vec4f(r);
}

struct SPIRV_Cross_Input
{
    @location(0) vertex : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) fragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vertex = stage_input.vertex;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.fragColor = fragColor;
    return stage_output;
}
