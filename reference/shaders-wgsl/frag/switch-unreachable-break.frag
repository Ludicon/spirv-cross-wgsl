struct UBO
{
    cond : i32,
    cond2 : i32,
}

@group(0) @binding(0) var<uniform> _13 : UBO;

var<private> FragColor : vec4f;
var<private> vInput : vec4f;

fn frag_main()
{
    var frog : bool = false;
    switch (_13.cond)
    {
        case 1:
        {
            if (_13.cond2 < 50)
            {
                break;
            }
            else
            {
                discard;
            }
            break; // unreachable workaround
        }
        default:
        {
            frog = true;
            break;
        }
    }
    FragColor = select(vec4f(20.0f), vec4f(10.0f), vec4<bool>(frog));
}

struct SPIRV_Cross_Input
{
    @location(0) vInput : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vInput = stage_input.vInput;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
