var<private> FragColor : i32;

fn frag_main()
{
    FragColor = 16;
    for (var i = 0; i < 25; i++)
    {
        FragColor += 10;
    }
    var i_1 = 1;
    var j = 4;
    loop
    {
        let _36 = i_1;
        let _38 = _36 < 30;
        if (_38)
        {
            FragColor += 11;
            i_1++;
            j += 4;
            continue;
        }
        else
        {
            break;
        }
    }
    var k = 0;
    for (; k < 20; k++)
    {
        FragColor += 12;
    }
    k += 3;
    FragColor += k;
    var l : i32;
    if (k == 40)
    {
        l = 0;
        for (; l < 40; l++)
        {
            FragColor += 13;
        }
        return;
    }
    else
    {
        l = k;
        FragColor += l;
    }
    var i_2 = vec2i(0);
    for (; i_2.x < 10; i_2.x += 4)
    {
        FragColor += i_2.y;
    }
    let o = k;
    for (var m = k; m < 40; m++)
    {
        FragColor += m;
    }
    FragColor += o;
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : i32,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
