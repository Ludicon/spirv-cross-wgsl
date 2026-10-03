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

@group(0) @binding(0) var<uniform> _22044 : _3;
@group(0) @binding(2) var<uniform> _12348 : _4;
@group(0) @binding(1) var<uniform> _15259 : _7;
@group(0) @binding(140) var _5785 : texture_2d<f32>;
@group(0) @binding(60) var _5688 : sampler;
@group(0) @binding(142) var _3312 : texture_2d<f32>;
@group(0) @binding(62) var _4646 : sampler;
@group(0) @binding(141) var _4862 : texture_2d<f32>;
@group(0) @binding(61) var _3594 : sampler;

var<private> gl_FragCoord : vec4f;
var<private> _4317 : vec4f;

fn frag_main()
{
    var _19927 : vec2f = gl_FragCoord.xy * _15259._m23.xy;
    var _17581 : vec4f = _22044._m2 * _22044._m0.xyxy;
    var _7011 : vec2f = _17581.xy;
    var _21058 : vec2f = _17581.zw;
    var _13149 : vec2f = clamp(_19927 + (vec2f(0.0f, -2.0f) * _22044._m0.xy), _7011, _21058);
    var _12103 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13149, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17670 : vec4f = textureSampleLevel(_3312, _4646, _13149, 0.0f);
    var _16938 : f32 = _17670.y;
    var _7719 : vec3f;
    if (_16938 > 0.0f)
    {
        _7719 = _12103 + (textureSampleLevel(_4862, _3594, _13149, 0.0f).xyz * clamp(_16938 * _17670.z, 0.0f, 1.0f));
    }
    else
    {
        _7719 = _12103;
    }
    var _13150 : vec2f = clamp(_19927 + (vec2f(-1.0f) * _22044._m0.xy), _7011, _21058);
    var _12104 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13150, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17671 : vec4f = textureSampleLevel(_3312, _4646, _13150, 0.0f);
    var _16939 : f32 = _17671.y;
    var _7720 : vec3f;
    if (_16939 > 0.0f)
    {
        _7720 = _12104 + (textureSampleLevel(_4862, _3594, _13150, 0.0f).xyz * clamp(_16939 * _17671.z, 0.0f, 1.0f));
    }
    else
    {
        _7720 = _12104;
    }
    var _13151 : vec2f = clamp(_19927 + (vec2f(0.0f, -1.0f) * _22044._m0.xy), _7011, _21058);
    var _12105 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13151, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17672 : vec4f = textureSampleLevel(_3312, _4646, _13151, 0.0f);
    var _16940 : f32 = _17672.y;
    var _7721 : vec3f;
    if (_16940 > 0.0f)
    {
        _7721 = _12105 + (textureSampleLevel(_4862, _3594, _13151, 0.0f).xyz * clamp(_16940 * _17672.z, 0.0f, 1.0f));
    }
    else
    {
        _7721 = _12105;
    }
    var _13152 : vec2f = clamp(_19927 + (vec2f(1.0f, -1.0f) * _22044._m0.xy), _7011, _21058);
    var _12106 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13152, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17673 : vec4f = textureSampleLevel(_3312, _4646, _13152, 0.0f);
    var _16941 : f32 = _17673.y;
    var _7722 : vec3f;
    if (_16941 > 0.0f)
    {
        _7722 = _12106 + (textureSampleLevel(_4862, _3594, _13152, 0.0f).xyz * clamp(_16941 * _17673.z, 0.0f, 1.0f));
    }
    else
    {
        _7722 = _12106;
    }
    var _13153 : vec2f = clamp(_19927 + (vec2f(-2.0f, 0.0f) * _22044._m0.xy), _7011, _21058);
    var _12107 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13153, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17674 : vec4f = textureSampleLevel(_3312, _4646, _13153, 0.0f);
    var _16942 : f32 = _17674.y;
    var _7723 : vec3f;
    if (_16942 > 0.0f)
    {
        _7723 = _12107 + (textureSampleLevel(_4862, _3594, _13153, 0.0f).xyz * clamp(_16942 * _17674.z, 0.0f, 1.0f));
    }
    else
    {
        _7723 = _12107;
    }
    var _13154 : vec2f = clamp(_19927 + (vec2f(-1.0f, 0.0f) * _22044._m0.xy), _7011, _21058);
    var _12108 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13154, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17675 : vec4f = textureSampleLevel(_3312, _4646, _13154, 0.0f);
    var _16943 : f32 = _17675.y;
    var _7724 : vec3f;
    if (_16943 > 0.0f)
    {
        _7724 = _12108 + (textureSampleLevel(_4862, _3594, _13154, 0.0f).xyz * clamp(_16943 * _17675.z, 0.0f, 1.0f));
    }
    else
    {
        _7724 = _12108;
    }
    var _13155 : vec2f = clamp(_19927, _7011, _21058);
    var _12109 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13155, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17676 : vec4f = textureSampleLevel(_3312, _4646, _13155, 0.0f);
    var _16944 : f32 = _17676.y;
    var _7725 : vec3f;
    if (_16944 > 0.0f)
    {
        _7725 = _12109 + (textureSampleLevel(_4862, _3594, _13155, 0.0f).xyz * clamp(_16944 * _17676.z, 0.0f, 1.0f));
    }
    else
    {
        _7725 = _12109;
    }
    var _13156 : vec2f = clamp(_19927 + (vec2f(1.0f, 0.0f) * _22044._m0.xy), _7011, _21058);
    var _12110 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13156, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17677 : vec4f = textureSampleLevel(_3312, _4646, _13156, 0.0f);
    var _16945 : f32 = _17677.y;
    var _7726 : vec3f;
    if (_16945 > 0.0f)
    {
        _7726 = _12110 + (textureSampleLevel(_4862, _3594, _13156, 0.0f).xyz * clamp(_16945 * _17677.z, 0.0f, 1.0f));
    }
    else
    {
        _7726 = _12110;
    }
    var _13157 : vec2f = clamp(_19927 + (vec2f(2.0f, 0.0f) * _22044._m0.xy), _7011, _21058);
    var _12111 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13157, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17678 : vec4f = textureSampleLevel(_3312, _4646, _13157, 0.0f);
    var _16946 : f32 = _17678.y;
    var _7727 : vec3f;
    if (_16946 > 0.0f)
    {
        _7727 = _12111 + (textureSampleLevel(_4862, _3594, _13157, 0.0f).xyz * clamp(_16946 * _17678.z, 0.0f, 1.0f));
    }
    else
    {
        _7727 = _12111;
    }
    var _13158 : vec2f = clamp(_19927 + (vec2f(-1.0f, 1.0f) * _22044._m0.xy), _7011, _21058);
    var _12112 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13158, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17679 : vec4f = textureSampleLevel(_3312, _4646, _13158, 0.0f);
    var _16947 : f32 = _17679.y;
    var _7728 : vec3f;
    if (_16947 > 0.0f)
    {
        _7728 = _12112 + (textureSampleLevel(_4862, _3594, _13158, 0.0f).xyz * clamp(_16947 * _17679.z, 0.0f, 1.0f));
    }
    else
    {
        _7728 = _12112;
    }
    var _13159 : vec2f = clamp(_19927 + (vec2f(0.0f, 1.0f) * _22044._m0.xy), _7011, _21058);
    var _12113 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13159, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17680 : vec4f = textureSampleLevel(_3312, _4646, _13159, 0.0f);
    var _16948 : f32 = _17680.y;
    var _7729 : vec3f;
    if (_16948 > 0.0f)
    {
        _7729 = _12113 + (textureSampleLevel(_4862, _3594, _13159, 0.0f).xyz * clamp(_16948 * _17680.z, 0.0f, 1.0f));
    }
    else
    {
        _7729 = _12113;
    }
    var _13160 : vec2f = clamp(_19927 + _22044._m0.xy, _7011, _21058);
    var _12114 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13160, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17681 : vec4f = textureSampleLevel(_3312, _4646, _13160, 0.0f);
    var _16949 : f32 = _17681.y;
    var _7730 : vec3f;
    if (_16949 > 0.0f)
    {
        _7730 = _12114 + (textureSampleLevel(_4862, _3594, _13160, 0.0f).xyz * clamp(_16949 * _17681.z, 0.0f, 1.0f));
    }
    else
    {
        _7730 = _12114;
    }
    var _13161 : vec2f = clamp(_19927 + (vec2f(0.0f, 2.0f) * _22044._m0.xy), _7011, _21058);
    var _12115 : vec3f = _12348._m5 * clamp(textureSampleLevel(_5785, _5688, _13161, 0.0f).w * _22044._m1, 0.0f, 1.0f);
    var _17682 : vec4f = textureSampleLevel(_3312, _4646, _13161, 0.0f);
    var _16950 : f32 = _17682.y;
    var _7731 : vec3f;
    if (_16950 > 0.0f)
    {
        _7731 = _12115 + (textureSampleLevel(_4862, _3594, _13161, 0.0f).xyz * clamp(_16950 * _17682.z, 0.0f, 1.0f));
    }
    else
    {
        _7731 = _12115;
    }
    var _13750 : vec3f = (((((((((((((_7719 * 0.5f).xyz + (_7720 * 0.5f)).xyz + (_7721 * 0.75f)).xyz + (_7722 * 0.5f)).xyz + (_7723 * 0.5f)).xyz + (_7724 * 0.75f)).xyz + (_7725 * 1.0f)).xyz + (_7726 * 0.75f)).xyz + (_7727 * 0.5f)).xyz + (_7728 * 0.5f)).xyz + (_7729 * 0.75f)).xyz + (_7730 * 0.5f)).xyz + (_7731 * 0.5f)).xyz * vec3f(0.125f);
    var _25050 : _15 = _15(vec4f(_13750.x, _13750.y, _13750.z, vec4f(0.0f).w));
    _25050._m0.w = 1.0f;
    _4317 = _25050._m0;
}

struct SPIRV_Cross_Input
{
    @builtin(position) gl_FragCoord : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) _4317 : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._4317 = _4317;
    return stage_output;
}
