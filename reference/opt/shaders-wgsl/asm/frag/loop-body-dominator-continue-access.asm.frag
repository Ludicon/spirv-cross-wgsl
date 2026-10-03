struct Foo
{
    lightVP : array<mat4x4f, 64>,
    shadowCascadesNum : u32,
    test : i32,
}

var<private> _239 : i32;

@group(0) @binding(0) var<uniform> _16 : Foo;

var<private> fragWorld : vec3f;
var<private> _entryPointOutput : i32;

fn frag_main()
{
    var _236 : i32;
    switch (0)
    {
        default:
        {
            var _233 : bool;
            var _237 : i32;
            var _230 = 0u;
            loop
            {
                if (_230 < _16.shadowCascadesNum)
                {
                    var _231 : mat4x4f;
                    switch (0)
                    {
                        default:
                        {
                            if (_16.test == 0)
                            {
                                _231 = mat4x4f(vec4f(0.5f, 0.0f, 0.0f, 0.0f), vec4f(0.0f, 0.5f, 0.0f, 0.0f), vec4f(0.0f, 0.0f, 0.5f, 0.0f), vec4f(0.0f, 0.0f, 0.0f, 1.0f));
                                break;
                            }
                            _231 = mat4x4f(vec4f(1.0f, 0.0f, 0.0f, 0.0f), vec4f(0.0f, 1.0f, 0.0f, 0.0f), vec4f(0.0f, 0.0f, 1.0f, 0.0f), vec4f(0.0f, 0.0f, 0.0f, 1.0f));
                            break;
                        }
                    }
                    let _178 = (_231 * transpose(_16.lightVP[_230])) * vec4f(fragWorld, 1.0f);
                    let _180 = _178.z;
                    let _187 = _178.x;
                    let _189 = _178.y;
                    if ((((_180 >= 0.0f) && (_180 <= 1.0f)) && (max(_187, _189) <= 1.0f)) && (min(_187, _189) >= 0.0f))
                    {
                        _237 = bitcast<i32>(_230);
                        _233 = true;
                        break;
                    }
                    _230 += bitcast<u32>(1);
                    continue;
                }
                else
                {
                    _237 = _239;
                    _233 = false;
                    break;
                }
            }
            if (_233)
            {
                _236 = _237;
                break;
            }
            _236 = -1;
            break;
        }
    }
    _entryPointOutput = _236;
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
