var<private> FragColor : vec4f;
var<private> counter : i32;
var<private> ucounter : u32;

fn frag_main()
{
    FragColor = vec4f(0.0f);
    var i : i32 = 0;
    var j : u32 = 1u;
    loop
    {
        var _23 : i32 = i;
        var _26 : bool = _23 < 10;
        var _27 : u32 = j;
        var _29 : bool = bitcast<i32>(_27) < bitcast<i32>(20u);
        var _30 : bool = _26 && _29;
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
