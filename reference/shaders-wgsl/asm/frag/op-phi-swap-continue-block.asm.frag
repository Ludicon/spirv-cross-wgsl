struct UBO
{
    uCount : i32,
    uJ : i32,
    uK : i32,
}

@group(0) @binding(0) var<uniform> _7 : UBO;

var<private> FragColor : f32;

fn frag_main()
{
    var _52 : i32;
    var _53 : i32;
    var _54 : i32;
    var _54_copy : i32;
    _54 = _7.uK;
    _53 = _7.uJ;
    _52 = 0;
    loop
    {
        let _31 = _52 < _7.uCount;
        if (_31)
        {
            _54_copy = _54;
            _54 = _53;
            _53 = _54_copy;
            _52++;
            continue;
        }
        else
        {
            break;
        }
    }
    FragColor = f32(_53 - _54) * f32(_7.uJ * _7.uK);
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
