struct MyStruct
{
    color : vec4f,
}

struct MyStruct_CB
{
    g_MyStruct : array<MyStruct, 4>,
}

@group(0) @binding(0) var<uniform> _8 : MyStruct_CB;

var<private> _entryPointOutput : vec4f;

fn frag_main()
{
    var _85 : vec3f;
    _85 = vec3f(0.0f);
    for (var _86 : i32 = 0; _86 < 4; )
    {
        _85 += _8.g_MyStruct[_86].color.xyz;
        _86++;
        continue;
    }
    _entryPointOutput = vec4f(_85, 1.0f);
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
