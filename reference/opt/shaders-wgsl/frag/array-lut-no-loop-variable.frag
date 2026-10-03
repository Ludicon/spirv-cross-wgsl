const _17 : array<f32, 5> = array<f32, 5>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f);

var<private> FragColor : vec4f;

fn frag_main()
{
    for (var _46 : i32 = 0; _46 < 4; )
    {
        var _33 : i32 = _46 + 1;
        FragColor += vec4f(_17[_33]);
        _46 = _33;
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
