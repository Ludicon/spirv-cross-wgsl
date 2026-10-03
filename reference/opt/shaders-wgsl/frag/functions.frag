const _49 : array<f32, 4> = array<f32, 4>(1.0f, 2.0f, 3.0f, 4.0f);

var<private> vColor : vec4f;
var<private> FragColor : vec4f;

fn frag_main()
{
    let _161 = vColor * 2.0f;
    let _166 = _161.x + _161.y;
    var _220 : f32;
    _220 = 0.0f;
    for (var _219 = 0; _219 < 4; )
    {
        _220 += _49[_219];
        _219++;
        continue;
    }
    FragColor = ((_161 + vec4f(normalize(vec3f(1.0f, 2.0f, 3.0f)) * 2.0f, _166)) + (vColor * 0.5f)) + vec4f(_166 * 2.0f, vec2f(_166) * 3.0f, _220);
}

struct SPIRV_Cross_Input
{
    @location(0) vColor : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vColor = stage_input.vColor;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
