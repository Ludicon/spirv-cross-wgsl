struct UBO
{
    count : i32,
    limit : i32,
}

@group(0) @binding(0) var<uniform> _60 : UBO;

var<private> vColor : vec4f;
var<private> vMode : i32;
var<private> FragColor : vec4f;

fn frag_main()
{
    var _200 : vec4f;
    var _202 : vec4f;
    switch (vMode)
    {
        case 0:
        {
            _202 = vColor * 2.0f;
            break;
        }
        case 1, 2:
        {
            _200 = vColor + vec4f(1.0f);
            let _32 = _200.wzyx;
            _202 = _32;
            break;
        }
        case 3:
        {
            _200 = vColor;
            let _32 = _200.wzyx;
            _202 = _32;
            break;
        }
        case -1:
        {
            discard;
        }
        default:
        {
            _202 = vec4f(0.0f);
            break;
        }
    }
    var _201 : vec4f;
    var _203 : i32;
    _203 = 0;
    _201 = _202;
    var _63 : i32;
    var _183 : vec4f;
    loop
    {
        _183 = _201;
        _183.x = _201.x + 0.100000001490116119384765625f;
        let _56 = _203 + 1;
        _63 = _60.count;
        if (_56 < _63)
        {
            _203 = _56;
            _201 = _183;
        }
        else
        {
            break;
        }
    }
    var _210 : vec4f;
    _210 = _183;
    var _204 = 0;
    var _205 = 10;
    for (; _204 < _205; )
    {
        var _186 = _210;
        _186.y = _210.y + f32(_204 * _205);
        _210 = _186;
        _205--;
        _204++;
        continue;
    }
    var _206 : i32;
    var _209 : vec4f;
    _209 = _210;
    _206 = 0;
    var _222 : vec4f;
    loop
    {
        let _99 = _206 < _63;
        if (_99)
        {
            if (_206 == _60.limit)
            {
                break;
            }
            if ((_206 & 1) != 0)
            {
                _222 = _209;
                let _153 = _206 + 1;
                _209 = _222;
                _206 = _153;
                continue;
            }
            var _208 : vec4f;
            _208 = _209;
            var _223 : vec4f;
            var _224 : vec4f;
            var _207 = 0;
            loop
            {
                if (_207 < _63)
                {
                    var _211 : vec4f;
                    switch (_207)
                    {
                        case 2:
                        {
                            _211 = _208;
                            break;
                        }
                        case 5:
                        {
                            var _192 = _208;
                            _192.z = _208.z + 1.0f;
                            _224 = _192;
                            let _151 = _207 + 1;
                            _208 = _224;
                            _207 = _151;
                            continue;
                        }
                        default:
                        {
                            var _189 = _208;
                            _189.w = _208.w - 0.5f;
                            _211 = _189;
                            break;
                        }
                    }
                    if (_211.w < (-10.0f))
                    {
                        _223 = _211;
                        break;
                    }
                    _224 = _211;
                    let _151 = _207 + 1;
                    _208 = _224;
                    _207 = _151;
                    continue;
                }
                else
                {
                    _223 = _208;
                    break;
                }
            }
            _222 = _223;
            let _153 = _206 + 1;
            _209 = _222;
            _206 = _153;
            continue;
        }
        else
        {
            break;
        }
    }
    var _219 : vec4f;
    _219 = _209;
    var _197 : vec4f;
    var _160 : f32;
    loop
    {
        _160 = _219.x;
        if (_160 < 100.0f)
        {
            _197 = _219;
            _197.x = _160 * 2.0f;
            _219 = _197;
            continue;
        }
        else
        {
            break;
        }
    }
    var _221 : vec4f;
    if (_160 > 50.0f)
    {
        _221 = _219;
    }
    else
    {
        _221 = _219.yxwz;
    }
    FragColor = _221;
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) vMode : i32,
    @location(1) vColor : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vMode = stage_input.vMode;
    vColor = stage_input.vColor;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
