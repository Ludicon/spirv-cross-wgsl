var<private> FragColor : f32;
var<private> vColor : f32;

fn func()
{
    FragColor = 1.0f;
    FragColor = 2.0f;
    if (vColor < 0.0f)
    {
        FragColor = 3.0f;
    }
    else
    {
        FragColor = 4.0f;
    }
    for (var i : i32 = 0; f32(i) < (40.0f + vColor); i += (i32(vColor) + 5))
    {
        FragColor += 0.20000000298023223876953125f;
        FragColor += 0.300000011920928955078125f;
    }
    switch (i32(vColor))
    {
        case 0:
        {
            FragColor += 0.20000000298023223876953125f;
            break;
        }
        case 1:
        {
            FragColor += 0.4000000059604644775390625f;
            break;
        }
        default:
        {
            FragColor += 0.800000011920928955078125f;
            break;
        }
    }
    loop
    {
        FragColor += (10.0f + vColor);
        continuing
        {
            break if !(FragColor < 100.0f);
        }
    }
}

fn frag_main()
{
    func();
}

struct SPIRV_Cross_Input
{
    @location(0) vColor : f32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vColor = stage_input.vColor;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
