struct Foo
{
    lightVP : array<mat4x4f, 64>,
    shadowCascadesNum : u32,
    test : i32,
}

@group(0) @binding(0) var<uniform> _16 : Foo;

var<private> fragWorld : vec3f;
var<private> _entryPointOutput : i32;

fn GetClip2TexMatrix() -> mat4x4f
{
    if (_16.test == 0)
    {
        return mat4x4f(vec4f(0.5f, 0.0f, 0.0f, 0.0f), vec4f(0.0f, 0.5f, 0.0f, 0.0f), vec4f(0.0f, 0.0f, 0.5f, 0.0f), vec4f(0.0f, 0.0f, 0.0f, 1.0f));
    }
    return mat4x4f(vec4f(1.0f, 0.0f, 0.0f, 0.0f), vec4f(0.0f, 1.0f, 0.0f, 0.0f), vec4f(0.0f, 0.0f, 1.0f, 0.0f), vec4f(0.0f, 0.0f, 0.0f, 1.0f));
}

fn GetCascade(fragWorldPosition : vec3f) -> i32
{
    for (var cascadeIndex = 0u; cascadeIndex < _16.shadowCascadesNum; cascadeIndex += bitcast<u32>(1))
    {
        let worldToShadowMap = GetClip2TexMatrix() * transpose(_16.lightVP[cascadeIndex]);
        let fragShadowMapPos = worldToShadowMap * vec4f(fragWorldPosition, 1.0f);
        if ((((fragShadowMapPos.z >= 0.0f) && (fragShadowMapPos.z <= 1.0f)) && (max(fragShadowMapPos.x, fragShadowMapPos.y) <= 1.0f)) && (min(fragShadowMapPos.x, fragShadowMapPos.y) >= 0.0f))
        {
            return bitcast<i32>(cascadeIndex);
        }
    }
    return -1;
}

fn _main(fragWorld_1 : vec3f) -> i32
{
    let param = fragWorld_1;
    return GetCascade(param);
}

fn frag_main()
{
    let fragWorld_1 = fragWorld;
    let param = fragWorld_1;
    _entryPointOutput = _main(param);
}

struct SPIRV_Cross_Input
{
    @location(0) fragWorld : vec3f,
}

struct SPIRV_Cross_Output
{
    @location(0) _entryPointOutput : i32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    fragWorld = stage_input.fragWorld;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._entryPointOutput = _entryPointOutput;
    return stage_output;
}
