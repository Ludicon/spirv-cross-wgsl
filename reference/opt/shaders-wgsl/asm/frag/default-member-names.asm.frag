var<private> _49 : f32;

var<private> _3 : vec4f;

fn frag_main()
{
    _3 = vec4f(_49);
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
