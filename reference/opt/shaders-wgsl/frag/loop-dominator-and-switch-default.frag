var<private> _80 : vec4f;

var<private> fragColor : vec4f;

fn frag_main()
{
    let _18 = i32(_80.x);
    var _81 : i32;
    var _82 : vec4f;
    _82 = _80;
    _81 = 0;
    var _89 : vec4f;
    loop
    {
        let _29 = _81 < _18;
        if (_29)
        {
            var _83 : vec4f;
            switch (_18)
            {
                case 0:
                {
                    var _74 = _82;
                    _74.y = 0.0f;
                    _83 = _74;
                    break;
                }
                case 1:
                {
                    var _76 = _82;
                    _76.y = 1.0f;
                    _83 = _76;
                    break;
                }
                default:
                {
                    var _88 : vec4f;
                    _88 = _82;
                    for (var _84 = 0; _84 < _18; )
                    {
                        var _72 = _88;
                        _72.y = _88.y + 0.5f;
                        _88 = _72;
                        _84++;
                        continue;
                    }
                    _89 = _88;
                    let _65 = _81 + 1;
                    _82 = _89;
                    _81 = _65;
                    continue;
                }
            }
            var _79 = _83;
            _79.y = _83.y + 0.5f;
            _89 = _79;
            let _65 = _81 + 1;
            _82 = _89;
            _81 = _65;
            continue;
        }
        else
        {
            break;
        }
    }
    fragColor = _82;
}

struct SPIRV_Cross_Output
{
    @location(0) fragColor : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.fragColor = fragColor;
    return stage_output;
}
