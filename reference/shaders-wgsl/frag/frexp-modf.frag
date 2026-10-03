struct ResType
{
    _m0 : f32,
    _m1 : i32,
}

struct ResType_1
{
    _m0 : vec2f,
    _m1 : vec2i,
}

struct ResType_2
{
    _m0 : f32,
    _m1 : f32,
}

struct ResType_3
{
    _m0 : vec2f,
    _m1 : vec2f,
}

var<private> v0 : f32;
var<private> v1 : vec2f;
var<private> FragColor : f32;

fn frag_main()
{
    let _16_frexp = frexp(v0);
    let _16 = ResType(_16_frexp.fract, _16_frexp.exp);
    var e0 = _16._m1;
    var f0 = _16._m0;
    let _22_frexp = frexp(v0 + 1.0f);
    let _22 = ResType(_22_frexp.fract, _22_frexp.exp);
    e0 = _22._m1;
    f0 = _22._m0;
    let _35_frexp = frexp(v1);
    let _35 = ResType_1(_35_frexp.fract, _35_frexp.exp);
    let e1 = _35._m1;
    let f1 = _35._m0;
    let _42_modf = modf(v0);
    let _42 = ResType_2(_42_modf.fract, _42_modf.whole);
    let r0 = _42._m1;
    let m0 = _42._m0;
    let _49_modf = modf(v1);
    let _49 = ResType_3(_49_modf.fract, _49_modf.whole);
    let r1 = _49._m1;
    let m1 = _49._m0;
    FragColor = ((((f0 + f1.x) + f1.y) + m0) + m1.x) + m1.y;
}

struct SPIRV_Cross_Input
{
    @location(0) v0 : f32,
    @location(1) v1 : vec2f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    v0 = stage_input.v0;
    v1 = stage_input.v1;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
