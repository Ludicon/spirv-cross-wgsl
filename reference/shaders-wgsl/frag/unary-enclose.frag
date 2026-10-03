var<private> FragColor : vec4f;
var<private> vIn : vec4f;
var<private> vIn1 : vec4i;

fn frag_main()
{
    FragColor = -(-vIn);
    var a : vec4i = ~(~vIn1);
    var b : bool = false;
    b = !(!b);
}

struct SPIRV_Cross_Input
{
    @location(0) vIn : vec4f,
    @location(1) @interpolate(flat) vIn1 : vec4i,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vIn = stage_input.vIn;
    vIn1 = stage_input.vIn1;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
