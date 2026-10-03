struct Foobar
{
    a : f32,
    b : f32,
}

const _37 : array<vec4f, 3> = array<vec4f, 3>(vec4f(1.0f), vec4f(2.0f), vec4f(3.0f));
const _55 : array<array<vec4f, 2>, 2> = array<array<vec4f, 2>, 2>(array<vec4f, 2>(vec4f(1.0f), vec4f(2.0f)), array<vec4f, 2>(vec4f(8.0f), vec4f(10.0f)));
const _75 : array<Foobar, 2> = array<Foobar, 2>(Foobar(10.0f, 40.0f), Foobar(90.0f, 70.0f));

var<private> FragColor : vec4f;
var<private> index : i32;

fn resolve(f : Foobar) -> vec4f
{
    return vec4f(f.a + f.b);
}

fn frag_main()
{
    var param : Foobar = Foobar(10.0f, 20.0f);
    var param_1 : Foobar = _75[index];
    FragColor = ((_37[index] + _55[index][index + 1]) + resolve(param)) + resolve(param_1);
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) index : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    index = stage_input.index;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
