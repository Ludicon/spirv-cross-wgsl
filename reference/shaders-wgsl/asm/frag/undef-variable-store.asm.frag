var<private> _48 : vec4f;
var<private> _31 : vec4f;

var<private> _entryPointOutput : vec4f;

fn frag_main()
{
    var _37 : vec4f;
    loop
    {
        var _35 : vec2f = vec2f(0.0f);
        if (_35.x != 0.0f)
        {
            _37 = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        }
        else
        {
            _37 = vec4f(1.0f, 1.0f, 0.0f, 1.0f);
            break;
        }
        _37 = _48;
        break;
        continuing
        {
            break if !false;
        }
    }
    _entryPointOutput = _37;
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
