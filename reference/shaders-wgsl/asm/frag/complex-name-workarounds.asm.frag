var<private> _4 : vec4f;
var<private> a : vec4f;
var<private> b : vec4f;

fn fu_nc_(a_ : vec4f) -> vec4f
{
    return a_;
}

fn fu_nc_11(_13 : vec4f) -> vec4f
{
    return _13;
}

fn frag_main()
{
    let b_1 = _4;
    let _14 = (_4 + a) + fu_nc_(b_1);
    let b_3 = a;
    let b_2 = (_4 - a) + fu_nc_11(b_3);
    b = _14;
    b = b_2;
    b = _14;
    b = b_2;
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
