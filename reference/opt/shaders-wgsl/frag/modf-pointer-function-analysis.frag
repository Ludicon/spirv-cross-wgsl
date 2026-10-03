struct ResType
{
    _m0 : vec4f,
    _m1 : vec4f,
}

struct ResType_1
{
    _m0 : f32,
    _m1 : f32,
}

var<private> v : vec4f;
var<private> vo0 : vec4f;
var<private> vo1 : vec4f;

fn frag_main()
{
    let _65_modf = modf(v);
    let _65 = ResType(_65_modf.fract, _65_modf.whole);
    vo0 = _65._m0;
    vo1 = _65._m1;
    let _73_modf = modf(v.x);
    let _73 = ResType_1(_73_modf.fract, _73_modf.whole);
    vo0.x += _73._m0;
    vo1.x += _73._m1;
}

struct SPIRV_Cross_Input
{
    @location(0) v : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) vo0 : vec4f,
    @location(1) vo1 : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    v = stage_input.v;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.vo0 = vo0;
    stage_output.vo1 = vo1;
    return stage_output;
}
