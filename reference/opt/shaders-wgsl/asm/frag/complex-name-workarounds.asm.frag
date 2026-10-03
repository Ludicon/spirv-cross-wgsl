var<private> _4 : vec4f;
var<private> a : vec4f;
var<private> b : vec4f;

fn frag_main()
{
    var _32 : vec4f = (_4 + a) + _4;
    b = _32;
    b = _4;
    b = _32;
    b = _4;
}

struct SPIRV_Cross_Input
{
    @location(0) _4 : vec4f,
    @location(1) a : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) b : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    _4 = stage_input._4;
    a = stage_input.a;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.b = b;
    return stage_output;
}
