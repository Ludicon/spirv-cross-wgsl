var<private> FragColor : vec4f;
var<private> vA : i32;
var<private> vB : i32;

fn frag_main()
{
    FragColor = vec4f(0.0f);
    var _57 : i32;
    var _60 : i32;
    _60 = 0;
    _57 = 0;
    var _49 : i32;
    var _58 : i32;
    var _25 : i32;
    loop
    {
        _25 = vA;
        let _27 = _57 < _25;
        if (_27)
        {
            if ((_25 + _57) == 20)
            {
                _58 = 50;
            }
            else
            {
                _58 = select(_60, 60, (vB + _57) == 40);
            }
            _49 = _58 + 10;
            FragColor += vec4f(1.0f);
            _60 = _58;
            _57 += _49;
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
    @location(0) @interpolate(flat) vA : i32,
    @location(1) @interpolate(flat) vB : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
