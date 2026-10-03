var<private> FragColor : vec4f;

fn frag_main()
{
    var written : bool = false;
    var v : f32;
    for (var j : i32 = 0; j < 10; j++)
    {
        for (var i : i32 = 0; i < 4; i++)
        {
            var w : f32 = 0.0f;
            if (written)
            {
                w += v;
            }
            else
            {
                v = 20.0f;
            }
            v += f32(i);
            written = true;
        }
    }
    FragColor = vec4f(1.0f);
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
