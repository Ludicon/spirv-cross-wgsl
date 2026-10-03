struct _15
{
    _m0 : vec4f,
}

struct _3
{
    _m0 : vec4f,
    _m1 : f32,
    _m2 : vec4f,
}

struct _4
{
    _m0 : vec3f,
    _m1 : vec3f,
    _m2 : f32,
    _m3 : vec3f,
    _m4 : f32,
    _m5 : vec3f,
    _m6 : f32,
    _m7 : vec3f,
    _m8 : f32,
    _m9 : vec3f,
    _m10 : f32,
    _m11 : vec3f,
    _m12 : f32,
    _m13 : vec2f,
    _m14 : vec2f,
    _m15 : vec3f,
    _m16 : f32,
    _m17 : f32,
    _m18 : f32,
    _m19 : f32,
    _m20 : f32,
    _m21 : vec4f,
    _m22 : vec4f,
    _m23 : mat4x4f,
    _m24 : vec4f,
}

struct _7
{
    _m0 : mat4x4f,
    _m1 : mat4x4f,
    _m2 : mat4x4f,
    _m3 : mat4x4f,
    _m4 : vec4f,
    _m5 : vec4f,
    _m6 : f32,
    _m7 : f32,
    _m8 : f32,
    _m9 : f32,
    _m10 : vec3f,
    _m11 : f32,
    _m12 : vec3f,
    _m13 : f32,
    _m14 : vec3f,
    _m15 : f32,
    _m16 : vec3f,
    _m17 : f32,
    _m18 : f32,
    _m19 : f32,
    _m20 : vec2f,
    _m21 : vec2f,
    _m22 : vec2f,
    _m23 : vec4f,
    _m24 : vec2f,
    _m25 : vec2f,
    _m26 : vec2f,
    _m27 : vec3f,
    _m28 : f32,
    _m29 : f32,
    _m30 : f32,
    _m31 : f32,
    _m32 : f32,
    _m33 : vec2f,
    _m34 : f32,
    _m35 : f32,
    _m36 : vec3f,
    _m37 : array<mat4x4f, 2>,
    _m38 : array<vec4f, 2>,
}

struct _8
{
    _m0 : vec4f,
    _m1 : vec4f,
    _m2 : vec2f,
    _m3 : vec2f,
    _m4 : vec3f,
    _m5 : f32,
    _m6 : vec3f,
    _m7 : f32,
    _m8 : vec4f,
    _m9 : vec4f,
    _m10 : vec4f,
    _m11 : vec3f,
    _m12 : f32,
    _m13 : vec3f,
    _m14 : f32,
    _m15 : vec3f,
    _m16 : vec4f,
    _m17 : vec3f,
    _m18 : f32,
    _m19 : vec3f,
    _m20 : vec2f,
}

struct _9
{
    _m0 : vec4f,
}

var<private> _10264 : _15;

@group(0) @binding(0) var<uniform> _22044 : _3;
@group(0) @binding(2) var<uniform> _12348 : _4;
@group(0) @binding(1) var<uniform> _15259 : _7;
@group(0) @binding(140) var _5785 : texture_2d<f32>;
@group(0) @binding(60) var _5688 : sampler;
@group(0) @binding(142) var _3312 : texture_2d<f32>;
@group(0) @binding(62) var _4646 : sampler;
@group(0) @binding(141) var _4862 : texture_2d<f32>;
@group(0) @binding(61) var _3594 : sampler;

var<private> _5800 : vec2f;
var<private> gl_FragCoord : vec4f;
var<private> _4317 : vec4f;

fn frag_main()
{
    var _13863 : _15;
    _13863._m0 = vec4f(0.0f);
    let _19927 = gl_FragCoord.xy * _15259._m23.xy;
    let _17581 = _22044._m2 * _22044._m0.xyxy;
    let _13149 = clamp(_19927 + (vec3f(0.0f, -2.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12103 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13149, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17670 = textureSampleLevel(_3312, _4646, _13149, 0.0f);
    var _7719 : vec3f;
    if (_17670.y > 0.0f)
    {
        _7719 = _12103 + (textureSampleLevel(_4862, _3594, _13149, 0.0f).xyz * clamp(_17670.y * _17670.z, 0.0f, 1.0f));
    }
    else
    {
        _7719 = _12103;
    }
    let _22177 = vec4f(0.0f).xyz + (_7719 * 0.5f);
    let _15527 = vec4f(_22177.x, _22177.y, _22177.z, vec4f(0.0f).w);
    _13863._m0 = _15527;
    let _13150 = clamp(_19927 + (vec3f(-1.0f, -1.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12104 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13150, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17671 = textureSampleLevel(_3312, _4646, _13150, 0.0f);
    var _7720 : vec3f;
    if (_17671.y > 0.0f)
    {
        _7720 = _12104 + (textureSampleLevel(_4862, _3594, _13150, 0.0f).xyz * clamp(_17671.y * _17671.z, 0.0f, 1.0f));
    }
    else
    {
        _7720 = _12104;
    }
    let _22178 = _15527.xyz + (_7720 * 0.5f);
    let _15528 = vec4f(_22178.x, _22178.y, _22178.z, _15527.w);
    _13863._m0 = _15528;
    let _13151 = clamp(_19927 + (vec3f(0.0f, -1.0f, 0.75f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12105 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13151, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17672 = textureSampleLevel(_3312, _4646, _13151, 0.0f);
    var _7721 : vec3f;
    if (_17672.y > 0.0f)
    {
        _7721 = _12105 + (textureSampleLevel(_4862, _3594, _13151, 0.0f).xyz * clamp(_17672.y * _17672.z, 0.0f, 1.0f));
    }
    else
    {
        _7721 = _12105;
    }
    let _22179 = _15528.xyz + (_7721 * 0.75f);
    let _15529 = vec4f(_22179.x, _22179.y, _22179.z, _15528.w);
    _13863._m0 = _15529;
    let _13152 = clamp(_19927 + (vec3f(1.0f, -1.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12106 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13152, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17673 = textureSampleLevel(_3312, _4646, _13152, 0.0f);
    var _7722 : vec3f;
    if (_17673.y > 0.0f)
    {
        _7722 = _12106 + (textureSampleLevel(_4862, _3594, _13152, 0.0f).xyz * clamp(_17673.y * _17673.z, 0.0f, 1.0f));
    }
    else
    {
        _7722 = _12106;
    }
    let _22180 = _15529.xyz + (_7722 * 0.5f);
    let _15530 = vec4f(_22180.x, _22180.y, _22180.z, _15529.w);
    _13863._m0 = _15530;
    let _13153 = clamp(_19927 + (vec3f(-2.0f, 0.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12107 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13153, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17674 = textureSampleLevel(_3312, _4646, _13153, 0.0f);
    var _7723 : vec3f;
    if (_17674.y > 0.0f)
    {
        _7723 = _12107 + (textureSampleLevel(_4862, _3594, _13153, 0.0f).xyz * clamp(_17674.y * _17674.z, 0.0f, 1.0f));
    }
    else
    {
        _7723 = _12107;
    }
    let _22181 = _15530.xyz + (_7723 * 0.5f);
    let _15531 = vec4f(_22181.x, _22181.y, _22181.z, _15530.w);
    _13863._m0 = _15531;
    let _13154 = clamp(_19927 + (vec3f(-1.0f, 0.0f, 0.75f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12108 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13154, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17675 = textureSampleLevel(_3312, _4646, _13154, 0.0f);
    var _7724 : vec3f;
    if (_17675.y > 0.0f)
    {
        _7724 = _12108 + (textureSampleLevel(_4862, _3594, _13154, 0.0f).xyz * clamp(_17675.y * _17675.z, 0.0f, 1.0f));
    }
    else
    {
        _7724 = _12108;
    }
    let _22182 = _15531.xyz + (_7724 * 0.75f);
    let _15532 = vec4f(_22182.x, _22182.y, _22182.z, _15531.w);
    _13863._m0 = _15532;
    let _13155 = clamp(_19927 + (vec3f(0.0f, 0.0f, 1.0f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12109 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13155, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17676 = textureSampleLevel(_3312, _4646, _13155, 0.0f);
    var _7725 : vec3f;
    if (_17676.y > 0.0f)
    {
        _7725 = _12109 + (textureSampleLevel(_4862, _3594, _13155, 0.0f).xyz * clamp(_17676.y * _17676.z, 0.0f, 1.0f));
    }
    else
    {
        _7725 = _12109;
    }
    let _22183 = _15532.xyz + (_7725 * 1.0f);
    let _15533 = vec4f(_22183.x, _22183.y, _22183.z, _15532.w);
    _13863._m0 = _15533;
    let _13156 = clamp(_19927 + (vec3f(1.0f, 0.0f, 0.75f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12110 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13156, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17677 = textureSampleLevel(_3312, _4646, _13156, 0.0f);
    var _7726 : vec3f;
    if (_17677.y > 0.0f)
    {
        _7726 = _12110 + (textureSampleLevel(_4862, _3594, _13156, 0.0f).xyz * clamp(_17677.y * _17677.z, 0.0f, 1.0f));
    }
    else
    {
        _7726 = _12110;
    }
    let _22184 = _15533.xyz + (_7726 * 0.75f);
    let _15534 = vec4f(_22184.x, _22184.y, _22184.z, _15533.w);
    _13863._m0 = _15534;
    let _13157 = clamp(_19927 + (vec3f(2.0f, 0.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12111 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13157, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17678 = textureSampleLevel(_3312, _4646, _13157, 0.0f);
    var _7727 : vec3f;
    if (_17678.y > 0.0f)
    {
        _7727 = _12111 + (textureSampleLevel(_4862, _3594, _13157, 0.0f).xyz * clamp(_17678.y * _17678.z, 0.0f, 1.0f));
    }
    else
    {
        _7727 = _12111;
    }
    let _22185 = _15534.xyz + (_7727 * 0.5f);
    let _15535 = vec4f(_22185.x, _22185.y, _22185.z, _15534.w);
    _13863._m0 = _15535;
    let _13158 = clamp(_19927 + (vec3f(-1.0f, 1.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12112 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13158, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17679 = textureSampleLevel(_3312, _4646, _13158, 0.0f);
    var _7728 : vec3f;
    if (_17679.y > 0.0f)
    {
        _7728 = _12112 + (textureSampleLevel(_4862, _3594, _13158, 0.0f).xyz * clamp(_17679.y * _17679.z, 0.0f, 1.0f));
    }
    else
    {
        _7728 = _12112;
    }
    let _22186 = _15535.xyz + (_7728 * 0.5f);
    let _15536 = vec4f(_22186.x, _22186.y, _22186.z, _15535.w);
    _13863._m0 = _15536;
    let _13159 = clamp(_19927 + (vec3f(0.0f, 1.0f, 0.75f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12113 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13159, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17680 = textureSampleLevel(_3312, _4646, _13159, 0.0f);
    var _7729 : vec3f;
    if (_17680.y > 0.0f)
    {
        _7729 = _12113 + (textureSampleLevel(_4862, _3594, _13159, 0.0f).xyz * clamp(_17680.y * _17680.z, 0.0f, 1.0f));
    }
    else
    {
        _7729 = _12113;
    }
    let _22187 = _15536.xyz + (_7729 * 0.75f);
    let _15537 = vec4f(_22187.x, _22187.y, _22187.z, _15536.w);
    _13863._m0 = _15537;
    let _13160 = clamp(_19927 + (vec3f(1.0f, 1.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12114 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13160, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17681 = textureSampleLevel(_3312, _4646, _13160, 0.0f);
    var _7730 : vec3f;
    if (_17681.y > 0.0f)
    {
        _7730 = _12114 + (textureSampleLevel(_4862, _3594, _13160, 0.0f).xyz * clamp(_17681.y * _17681.z, 0.0f, 1.0f));
    }
    else
    {
        _7730 = _12114;
    }
    let _22188 = _15537.xyz + (_7730 * 0.5f);
    let _15539 = vec4f(_22188.x, _22188.y, _22188.z, _15537.w);
    _13863._m0 = _15539;
    let _13161 = clamp(_19927 + (vec3f(0.0f, 2.0f, 0.5f).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    let _12115 = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13161, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    let _17682 = textureSampleLevel(_3312, _4646, _13161, 0.0f);
    var _7731 : vec3f;
    if (_17682.y > 0.0f)
    {
        _7731 = _12115 + (textureSampleLevel(_4862, _3594, _13161, 0.0f).xyz * clamp(_17682.y * _17682.z, 0.0f, 1.0f));
    }
    else
    {
        _7731 = _12115;
    }
    let _22189 = _15539.xyz + (_7731 * 0.5f);
    let _15541 = vec4f(_22189.x, _22189.y, _22189.z, _15539.w);
    _13863._m0 = _15541;
    let _13750 = _15541.xyz / vec3f(((((((((((((0.0f + 0.5f) + 0.5f) + 0.75f) + 0.5f) + 0.5f) + 0.75f) + 1.0f) + 0.75f) + 0.5f) + 0.5f) + 0.75f) + 0.5f) + 0.5f);
    _13863._m0 = vec4f(_13750.x, _13750.y, _13750.z, _15541.w);
    _13863._m0.w = 1.0f;
    _4317 = _13863._m0;
}

struct SPIRV_Cross_Input
{
    @location(0) _5800 : vec2f,
    @builtin(position) gl_FragCoord : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) _4317 : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    _5800 = stage_input._5800;
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._4317 = _4317;
    return stage_output;
}
