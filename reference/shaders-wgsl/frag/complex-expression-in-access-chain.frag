struct UBO
{
    results : array<vec4f, 1024>,
}

@group(0) @binding(0) var<storage, read_write> _34 : UBO;
@group(0) @binding(1) var Buf : texture_2d<i32>;
@group(0) @binding(17) var Buf_sampler : sampler;

var<private> gl_FragCoord : vec4f;
var<private> vIn : i32;
var<private> vIn2 : i32;
var<private> FragColor : vec4f;

fn frag_main()
{
    var coords : vec4i = textureLoad(Buf, vec2i(gl_FragCoord.xy), 0);
    var foo : vec4f = _34.results[coords.x % 16];
    var c : i32 = vIn * vIn;
    var d : i32 = vIn2 * vIn2;
    FragColor = (foo + foo) + _34.results[c + d];
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) vIn : i32,
    @location(1) @interpolate(flat) vIn2 : i32,
    @builtin(position) gl_FragCoord : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vIn = stage_input.vIn;
    vIn2 = stage_input.vIn2;
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
