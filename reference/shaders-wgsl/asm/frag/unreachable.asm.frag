var<private> _44 : vec4f;

var<private> counter : i32;
var<private> FragColor : vec4f;

fn frag_main()
{
    var _45 : vec4f;
    _45 = _44;
    var _46 : vec4f;
    loop
    {
        if (counter == 10)
        {
            _46 = vec4f(10.0f);
            break;
        }
        else
        {
            _46 = vec4f(30.0f);
            break;
        }
    }
    FragColor = _46;
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) counter : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    counter = stage_input.counter;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
