struct V2F
{
    Position : vec4f,
    Color : vec4f,
}

struct InstanceData
{
    MATRIX_MVP : mat4x4f,
    Color : vec4f,
}

struct InstanceData_1
{
    MATRIX_MVP : mat4x4f,
    Color : vec4f,
}

struct gInstanceData
{
    _data : array<InstanceData_1>,
}

@group(1) @binding(0) var<storage, read> gInstanceData_1 : gInstanceData;

var<private> PosL : vec3f;
var<private> gl_InstanceIndex : u32;
var<private> gl_Position : vec4f;
var<private> _entryPointOutput_Color : vec4f;

fn _VS(PosL_1 : vec3f, instanceID : u32) -> V2F
{
    var instData : InstanceData;
    instData.MATRIX_MVP = transpose(gInstanceData_1._data[instanceID].MATRIX_MVP);
    instData.Color = gInstanceData_1._data[instanceID].Color;
    var v2f : V2F;
    v2f.Position = instData.MATRIX_MVP * vec4f(PosL_1, 1.0f);
    v2f.Color = instData.Color;
    return v2f;
}

fn vert_main()
{
    let PosL_1 = PosL;
    let instanceID = bitcast<u32>(gl_InstanceIndex);
    let param = PosL_1;
    let param_1 = instanceID;
    let flattenTemp = _VS(param, param_1);
    gl_Position = flattenTemp.Position;
    _entryPointOutput_Color = flattenTemp.Color;
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
