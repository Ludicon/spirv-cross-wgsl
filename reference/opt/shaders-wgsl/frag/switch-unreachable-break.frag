struct UBO
{
    cond : i32,
    cond2 : i32,
}

@group(0) @binding(0) var<uniform> _13 : UBO;

var<private> FragColor : vec4f;

fn frag_main()
{
    var _49 : bool;
    switch (_13.cond)
    {
        case 1:
        {
            if (_13.cond2 < 50)
            {
                _49 = false;
                break;
            }
            else
            {
                discard;
            }
        }
        default:
        {
            _49 = true;
            break;
        }
    }
    FragColor = select(vec4f(20.0f), vec4f(10.0f), vec4<bool>(_49));
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
