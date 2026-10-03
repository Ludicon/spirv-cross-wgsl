diagnostic(off, derivative_uniformity);

struct GlobalPSData
{
    g_CamPos : vec4f,
    g_SunDir : vec4f,
    g_SunColor : vec4f,
    g_ResolutionParams : vec4f,
    g_TimeParams : vec4f,
    g_FogColor_Distance : vec4f,
}

@group(0) @binding(4) var<uniform> _101 : GlobalPSData;
@group(0) @binding(2) var TexNormalmap : texture_2d<f32>;
@group(0) @binding(18) var TexNormalmap_sampler : sampler;

var<private> LightingOut : vec4f;
var<private> NormalOut : vec4f;
var<private> SpecularOut : vec4f;
var<private> AlbedoOut : vec4f;
var<private> TexCoord : vec2f;
var<private> EyeVec : vec3f;

fn frag_main()
{
    var _68 : vec3f = normalize((textureSample(TexNormalmap, TexNormalmap_sampler, TexCoord).xyz * 2.0f) - vec3f(1.0f));
    var _113 : f32 = smoothstep(0.0f, 0.1500000059604644775390625f, (_101.g_CamPos.y + EyeVec.y) * 0.004999999888241291046142578125f);
    var _125 : f32 = smoothstep(0.699999988079071044921875f, 0.75f, _68.y);
    var _130 : vec3f = mix(vec3f(0.100000001490116119384765625f), mix(vec3f(0.100000001490116119384765625f, 0.300000011920928955078125f, 0.100000001490116119384765625f), vec3f(0.800000011920928955078125f), vec3f(_113)), vec3f(_125));
    LightingOut = vec4f(0.0f);
    NormalOut = vec4f((_68 * 0.5f) + vec3f(0.5f), 0.0f);
    SpecularOut = vec4f(1.0f - (_125 * _113), 0.0f, 0.0f, 0.0f);
    AlbedoOut = vec4f(_130 * _130, 1.0f);
}

struct SPIRV_Cross_Input
{
    @location(0) TexCoord : vec2f,
    @location(1) EyeVec : vec3f,
}

struct SPIRV_Cross_Output
{
    @location(0) AlbedoOut : vec4f,
    @location(1) SpecularOut : vec4f,
    @location(2) NormalOut : vec4f,
    @location(3) LightingOut : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    TexCoord = stage_input.TexCoord;
    EyeVec = stage_input.EyeVec;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.AlbedoOut = AlbedoOut;
    stage_output.SpecularOut = SpecularOut;
    stage_output.NormalOut = NormalOut;
    stage_output.LightingOut = LightingOut;
    return stage_output;
}
