var<private> FragColor : vec4f;

fn frag_main()
{
    var _47 : i32;
    var _50 : f32;
    _50 = 0.0f;
    _47 = 0;
    var _25 : f32;
    var _28 : i32;
    loop
    {
        var _22 : bool = _47 < 16;
        if (_22)
        {
            _25 = _50 + 1.0f;
            _28 = _47 + 1;
            _50 = _25;
            _47 = _28;
            continue;
        }
        else
        {
            break;
        }
    }
    FragColor = vec4f(_50);
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
