diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var uShadow2DArray : texture_depth_2d_array;
@group(0) @binding(16) var uShadow2DArray_sampler : sampler_comparison;

var<private> vUV : vec4f;
var<private> vBias : f32;
var<private> FragColor : vec4f;

fn frag_main()
{
    var r = 0.0f;
    r += textureSampleCompare(uShadow2DArray, uShadow2DArray_sampler, vUV.xy, i32(round(vUV.z)), vUV.w);
    r += textureSampleCompare(uShadow2DArray, uShadow2DArray_sampler, vUV.xy, i32(round(vUV.z)), vUV.w, vec2i(1));
    FragColor = vec4f(r);
}

struct SPIRV_Cross_Input
{
    @location(0) vUV : vec4f,
    @location(1) vBias : f32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vUV = stage_input.vUV;
    vBias = stage_input.vBias;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
