struct InstanceData
{
    MATRIX_MVP : mat4x4f,
    Color : vec4f,
}

struct gInstanceData
{
    _data : array<InstanceData>,
}

@group(1) @binding(0) var<storage, read> gInstanceData_1 : gInstanceData;

var<private> PosL : vec3f;
var<private> gl_InstanceIndex : u32;
var<private> gl_Position : vec4f;
var<private> _entryPointOutput_Color : vec4f;

fn vert_main()
{
    gl_Position = vec4f(PosL, 1.0f) * gInstanceData_1._data[bitcast<u32>(gl_InstanceIndex)].MATRIX_MVP;
    _entryPointOutput_Color = gInstanceData_1._data[bitcast<u32>(gl_InstanceIndex)].Color;
}

struct SPIRV_Cross_Input
{
    @location(0) PosL : vec3f,
    @builtin(instance_index) gl_InstanceIndex : u32,
}

struct SPIRV_Cross_Output
{
    @location(0) _entryPointOutput_Color : vec4f,
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    PosL = stage_input.PosL;
    gl_InstanceIndex = stage_input.gl_InstanceIndex;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output._entryPointOutput_Color = _entryPointOutput_Color;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
