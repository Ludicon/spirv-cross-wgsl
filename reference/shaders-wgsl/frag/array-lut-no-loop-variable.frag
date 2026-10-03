const _17 : array<f32, 5> = array<f32, 5>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f);

var<private> FragColor : vec4f;
var<private> v0 : vec4f;

fn frag_main()
{
    var i = 0;
    loop
    {
        let _27 = i;
        let _30 = _27 < 4;
        if (_30)
        {
            i++;
            FragColor += vec4f(_17[i]);
            continue;
        }
        else
        {
            break;
        }
    }
}

struct SPIRV_Cross_Input
{
    @location(0) v0 : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    v0 = stage_input.v0;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
