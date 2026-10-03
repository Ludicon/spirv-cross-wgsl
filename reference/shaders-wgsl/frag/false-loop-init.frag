var<private> result : vec4f;
var<private> accum : vec4f;

fn frag_main()
{
    result = vec4f(0.0f);
    var j : u32;
    for (var i : i32 = 0; i < 4; i += bitcast<i32>(j))
    {
        if (accum.y > 10.0f)
        {
            j = 40u;
        }
        else
        {
            j = 30u;
        }
        result += accum;
    }
}

struct SPIRV_Cross_Input
{
    @location(0) accum : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) result : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    accum = stage_input.accum;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.result = result;
    return stage_output;
}
