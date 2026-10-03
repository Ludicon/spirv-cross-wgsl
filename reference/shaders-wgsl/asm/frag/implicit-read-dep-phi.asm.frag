@group(0) @binding(0) var uImage : texture_2d<f32>;
@group(0) @binding(16) var uImage_sampler : sampler;

var<private> v0 : vec4f;
var<private> FragColor : vec4f;

fn frag_main()
{
    var i : i32 = 0;
    var phi : f32;
    var _45 : vec4f;
    phi = 1.0f;
    _45 = vec4f(1.0f, 2.0f, 1.0f, 2.0f);
    loop
    {
        FragColor = _45;
        if (i < 4)
        {
            if (v0[i] > 0.0f)
            {
                var _43 : vec2f = vec2f(phi);
                i++;
                phi += 2.0f;
                _45 = textureSampleLevel(uImage, uImage_sampler, _43, 0.0f);
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

struct SPIRV_Cross_Input
{
    @location(0) v0 : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    v0 = stage_input.v0;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
