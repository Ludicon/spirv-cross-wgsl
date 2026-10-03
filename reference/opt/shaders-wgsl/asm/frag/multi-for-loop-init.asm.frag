var<private> FragColor : vec4f;
var<private> counter : i32;

fn frag_main()
{
    FragColor = vec4f(0.0f);
    var _53 = 0;
    var _54 = 1u;
    for (; (_53 < 10) && (bitcast<i32>(_54) < bitcast<i32>(20u)); )
    {
        FragColor += vec4f(f32(_53));
        FragColor += vec4f(f32(_54));
        _54 += bitcast<u32>(counter);
        _53 += counter;
        continue;
    }
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) counter : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    counter = stage_input.counter;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
