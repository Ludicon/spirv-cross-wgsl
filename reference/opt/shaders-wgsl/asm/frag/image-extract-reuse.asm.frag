@group(0) @binding(0) var uTexture : texture_2d<f32>;
@group(0) @binding(16) var uTexture_sampler : sampler;

var<private> Size : vec2i;

fn frag_main()
{
    Size = vec2i(textureDimensions(uTexture, 0)) + vec2i(textureDimensions(uTexture, 1));
}

struct SPIRV_Cross_Output
{
    @location(0) Size : vec2i,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.Size = Size;
    return stage_output;
}
