var<private> vIndex : i32;
var<private> FragColor : f32;

fn frag_main()
{
    var _27 : f32;
    switch (vIndex)
    {
        case 0, 2:
        {
            _27 = 1.0f;
            break;
        }
        default:
        {
            _27 = 3.0f;
            break;
        }
        case 8:
        {
            _27 = 8.0f;
            break;
        }
    }
    FragColor = _27;
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) vIndex : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vIndex = stage_input.vIndex;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
