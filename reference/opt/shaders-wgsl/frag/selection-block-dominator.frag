var<private> vIndex : i32;
var<private> FragColor : vec4f;

fn frag_main()
{
    switch (0)
    {
        default:
        {
            if (vIndex != 1)
            {
                FragColor = vec4f(1.0f);
                break;
            }
            FragColor = vec4f(10.0f);
            break;
        }
    }
}

struct SPIRV_Cross_Input
{
    @location(0) @interpolate(flat) vIndex : i32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vIndex = stage_input.vIndex;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
