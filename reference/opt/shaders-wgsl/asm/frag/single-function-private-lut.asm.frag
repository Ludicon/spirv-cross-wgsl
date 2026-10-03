struct myType
{
    data : f32,
}

const _21 : array<myType, 5> = array<myType, 5>(myType(0.0f), myType(1.0f), myType(0.0f), myType(1.0f), myType(0.0f));

var<private> gl_FragCoord : vec4f;
var<private> o_color : vec4f;

fn frag_main()
{
    if (_21[i32(gl_FragCoord.x - 4.0f * floor(gl_FragCoord.x / 4.0f))].data > 0.0f)
    {
        o_color = vec4f(0.0f, 1.0f, 0.0f, 1.0f);
    }
    else
    {
        o_color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
    }
}

struct SPIRV_Cross_Input
{
    @builtin(position) gl_FragCoord : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) o_color : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.o_color = o_color;
    return stage_output;
}
