struct Buffer
{
    HP : mat4x4f,
    MP : mat4x4f,
}

struct Buffer2
{
    MP2 : mat4x4f,
}

@group(0) @binding(0) var<uniform> _21 : Buffer;
@group(0) @binding(1) var<uniform> _39 : Buffer2;

var<private> gl_Position : vec4f;
var<private> H : vec4f;
var<private> Hin : vec4f;
var<private> M : vec4f;
var<private> Min : vec4f;
var<private> M2 : vec4f;

fn vert_main()
{
    gl_Position = vec4f(1.0f);
    H = Hin * _21.HP;
    M = Min * _21.MP;
    M2 = Min * _39.MP2;
}

struct SPIRV_Cross_Input
{
    @location(0) Hin : vec4f,
    @location(1) Min : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) H : vec4f,
    @location(1) M : vec4f,
    @location(2) M2 : vec4f,
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    Hin = stage_input.Hin;
    Min = stage_input.Min;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.H = H;
    stage_output.M = M;
    stage_output.M2 = M2;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
