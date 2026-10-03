struct _10
{
    _m0 : f32,
}

struct _11
{
    _m0 : f32,
    _m1 : f32,
    _m2 : f32,
    _m3 : f32,
    _m4 : f32,
    _m5 : f32,
    _m6 : f32,
    _m7 : f32,
    _m8 : f32,
    _m9 : f32,
    _m10 : f32,
    _m11 : f32,
    _m12 : _10,
}

var<private> _3 : vec4f;

fn frag_main()
{
    var _23 : _11;
    _3 = vec4f(_23._m0, _23._m1, _23._m2, _23._m3);
}

struct SPIRV_Cross_Output
{
    @location(0) _3 : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._3 = _3;
    return stage_output;
}
