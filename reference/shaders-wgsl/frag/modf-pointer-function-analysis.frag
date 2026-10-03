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

fn modf_inner(tmp : ptr<function, vec4f>) -> vec4f
{
    let _21_modf = modf(v);
    var _21 : ResType = ResType(_21_modf.fract, _21_modf.whole);
    (*tmp) = _21._m1;
    return _21._m0;
}

fn modf_inner_partial(tmp : ptr<function, vec4f>) -> f32
{
    let _34_modf = modf(v.x);
    var _34 : ResType_1 = ResType_1(_34_modf.fract, _34_modf.whole);
    (*tmp).x = _34._m1;
    return _34._m0;
}

fn frag_main()
{
    var param : vec4f;
    var _43 : vec4f = modf_inner(&param);
    var tmp : vec4f = param;
    vo0 = _43;
    vo1 = tmp;
    var param_1 : vec4f = tmp;
    var _49 : f32 = modf_inner_partial(&param_1);
    tmp = param_1;
    vo0.x += _49;
    vo1.x += tmp.x;
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
