struct Buff
{
    TestVal : u32,
}

@group(0) @binding(0) var<uniform> _15 : Buff;

var<private> fsout_Color : vec4f;

fn frag_main()
{
    fsout_Color = vec4f(1.0f);
    switch (_15.TestVal)
    {
        case 0u:
        {
            fsout_Color = vec4f(0.100000001490116119384765625f);
            break;
        }
        case 1u:
        {
            fsout_Color = vec4f(0.20000000298023223876953125f);
            break;
        }
        default:
        {
            break;
        }
    }
}

struct SPIRV_Cross_Output
{
    @location(0) fsout_Color : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.fsout_Color = fsout_Color;
    return stage_output;
}
