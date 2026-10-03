var<private> FragColor : vec4f;
var<private> vA : i32;
var<private> vB : i32;

fn frag_main()
{
    FragColor = vec4f(0.0f);
    var k : i32 = 0;
    var j : i32;
    for (var i : i32 = 0; i < vA; i += j)
    {
        if ((vA + i) == 20)
        {
            k = 50;
        }
        else
        {
            if ((vB + i) == 40)
            {
                k = 60;
            }
        }
        j = k + 10;
        FragColor += vec4f(1.0f);
    }
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) vA : i32,
    @location(1) @interpolate(flat) vB : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
