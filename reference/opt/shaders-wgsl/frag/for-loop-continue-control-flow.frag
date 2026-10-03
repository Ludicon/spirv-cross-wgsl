var<private> FragColor : vec4f;

fn frag_main()
{
    FragColor = vec4f(0.0f);
    for (var _43 = 0; _43 < 3; )
    {
        FragColor[_43] += f32(_43);
        _43++;
        continue;
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
