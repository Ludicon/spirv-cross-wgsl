var<private> FragColor : vec4f;

fn frag_main()
{
    FragColor = vec4f(0.0f);
    var i : i32 = 0;
    var _36 : i32;
    loop
    {
        if (i < 3)
        {
            var a : i32 = i;
            FragColor[a] += f32(i);
            if (false)
            {
                _36 = 1;
            }
            else
            {
                var _41 : i32 = i;
                i = _41 + 1;
                _36 = _41;
            }
            continue;
        }
        else
        {
            break;
        }
    }
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
