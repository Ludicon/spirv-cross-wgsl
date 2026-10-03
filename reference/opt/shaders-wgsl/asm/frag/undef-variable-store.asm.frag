var<private> _entryPointOutput : vec4f;

fn frag_main()
{
    _entryPointOutput = vec4f(1.0f, 1.0f, 0.0f, 1.0f);
}

struct SPIRV_Cross_Output
{
    @location(0) _entryPointOutput : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._entryPointOutput = _entryPointOutput;
    return stage_output;
}
