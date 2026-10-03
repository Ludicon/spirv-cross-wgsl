var<private> FragColor : vec4f;
var<private> vIn0 : vec4f;
var<private> vIn1 : vec4f;
var<private> vIn2 : f32;
var<private> vIn3 : f32;

fn frag_main()
{
    FragColor = vec4f(vIn0.x, vIn1.y, vIn0.z, vIn0.w);
    FragColor = vec4f(vIn3);
    FragColor = vIn0.xyzw;
    FragColor = vec4f(vIn2);
}

struct SPIRV_Cross_Input
{
    @location(0) vIn0 : vec4f,
    @location(1) vIn1 : vec4f,
    @location(2) vIn2 : f32,
    @location(3) vIn3 : f32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vIn0 = stage_input.vIn0;
    vIn1 = stage_input.vIn1;
    vIn2 = stage_input.vIn2;
    vIn3 = stage_input.vIn3;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
