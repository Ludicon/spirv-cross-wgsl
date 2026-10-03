var<private> FragColor : vec4f;

fn frag_main()
{
    var _50 : f32;
    _50 = 0.0f;
    for (var _47 : i32 = 0; _47 < 16; )
    {
        _50 += 1.0f;
        _47++;
        continue;
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
