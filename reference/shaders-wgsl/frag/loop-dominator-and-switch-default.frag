var<private> fragColor : vec4f;

fn frag_main()
{
    var f4 : vec4f;
    var c : i32 = i32(f4.x);
    for (var j : i32 = 0; j < c; j++)
    {
        switch (c)
        {
            case 0:
            {
                f4.y = 0.0f;
                break;
            }
            case 1:
            {
                f4.y = 1.0f;
                break;
            }
            default:
            {
                var i : i32 = 0;
                loop
                {
                    var _48 : i32 = i;
                    var _50 : i32 = _48 + 1;
                    i = _50;
                    if (_48 < c)
                    {
                        f4.y += 0.5f;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                continue;
            }
        }
        f4.y += 0.5f;
    }
    fragColor = f4;
}

struct SPIRV_Cross_Output
{
    @location(0) fragColor : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.fragColor = fragColor;
    return stage_output;
}
