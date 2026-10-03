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
    var c = vColor;
    switch (vMode)
    {
        case 0:
        {
            c *= 2.0f;
            break;
        }
        case 1, 2:
        {
            c += vec4f(1.0f);
            let _31 = c;
            c = _31.wzyx;
            break;
        }
        case 3:
        {
            let _31 = c;
            c = _31.wzyx;
            break;
        }
        case -1:
        {
            discard;
        }
        default:
        {
            c = vec4f(0.0f);
            break;
        }
    }
    var i = 0;
    loop
    {
        c.x += 0.100000001490116119384765625f;
        i++;
        continuing
        {
            break if !(i < _60.count);
        }
    }
    var j = 0;
    var k = 10;
    loop
    {
        let _74 = j;
        let _75 = k;
        let _76 = _74 < _75;
        if (_76)
        {
            c.y += f32(j * k);
            j++;
            k--;
            continue;
        }
        else
        {
            break;
        }
    }
    for (var j_1 = 0; j_1 < _60.count; j_1++)
    {
        if (j_1 == _60.limit)
        {
            break;
        }
        if ((j_1 & 1) != 0)
        {
            continue;
        }
        for (var k_1 = 0; k_1 < _60.count; k_1++)
        {
            switch (k_1)
            {
                case 2:
                {
                    break;
                }
                case 5:
                {
                    c.z += 1.0f;
                    continue;
                }
                default:
                {
                    c.w -= 0.5f;
                    break;
                }
            }
            if (c.w < (-10.0f))
            {
                break;
            }
        }
    }
    while (c.x < 100.0f)
    {
        c.x *= 2.0f;
    }
    var _173 : vec4f;
    if (c.x > 50.0f)
    {
        _173 = c;
    }
    else
    {
        _173 = c.yxwz;
    }
    FragColor = _173;
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
