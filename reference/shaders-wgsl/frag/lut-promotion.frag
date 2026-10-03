const _16 : array<f32, 16> = array<f32, 16>(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
const _60 : array<vec4f, 4> = array<vec4f, 4>(vec4f(0.0f), vec4f(1.0f), vec4f(8.0f), vec4f(5.0f));

var<private> FragColor : f32;
var<private> index : i32;

fn frag_main()
{
    FragColor = _16[index];
    if (index < 10)
    {
        FragColor += _16[index ^ 1];
    }
    else
    {
        FragColor += _16[index & 1];
    }
    if (index > 30)
    {
        FragColor += _60[index & 3].y;
    }
    else
    {
        FragColor += _60[index & 1].x;
    }
    var foobar : array<vec4f, 4> = _60;
    if (index > 30)
    {
        foobar[1].z = 20.0f;
    }
    FragColor += foobar[index & 3].z;
    var baz : array<vec4f, 4> = _60;
    baz = array<vec4f, 4>(vec4f(20.0f), vec4f(30.0f), vec4f(50.0f), vec4f(60.0f));
    FragColor += baz[index & 3].z;
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) index : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
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
