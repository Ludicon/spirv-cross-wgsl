var<private> FragColor : vec4f;
var<private> counter : i32;
var<private> ucounter : u32;

fn frag_main()
{
    FragColor = vec4f(0.0f);
    var i = 0;
    var j = 1u;
    loop
    {
        let _23 = i;
        let _26 = _23 < 10;
        let _27 = j;
        let _29 = bitcast<i32>(_27) < bitcast<i32>(20u);
        let _30 = _26 && _29;
        if (_30)
        {
            FragColor += vec4f(f32(i));
            FragColor += vec4f(f32(j));
            i += counter;
            j += bitcast<u32>(counter);
            continue;
        }
        else
        {
            break;
        }
    }
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) counter : i32,
    @location(1) @interpolate(flat) ucounter : u32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    counter = stage_input.counter;
    ucounter = stage_input.ucounter;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
