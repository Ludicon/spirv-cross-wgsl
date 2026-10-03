var<private> FragColor : f32;

fn frag_main()
{
    var foo : f32 = 1.0f;
    loop
    {
        foo = 2.0f;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    loop
    {
        foo = 3.0f;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    loop
    {
        foo = 4.0f;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    loop
    {
        foo = 5.0f;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    FragColor = foo;
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
