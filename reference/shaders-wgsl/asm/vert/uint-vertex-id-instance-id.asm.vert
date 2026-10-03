var<private> gl_VertexIndex : u32;
var<private> gl_InstanceIndex : u32;
var<private> gl_Position : vec4f;

fn _main(vid : u32, iid : u32) -> vec4f
{
    return vec4f(f32(vid + iid));
}

fn vert_main()
{
    var vid : u32 = bitcast<u32>(gl_VertexIndex);
    var iid : u32 = bitcast<u32>(gl_InstanceIndex);
    var param : u32 = vid;
    var param_1 : u32 = iid;
    gl_Position = _main(param, param_1);
}

struct SPIRV_Cross_Input
{
    @builtin(vertex_index) gl_VertexIndex : u32,
    @builtin(instance_index) gl_InstanceIndex : u32,
}

struct SPIRV_Cross_Output
{
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    gl_VertexIndex = stage_input.gl_VertexIndex;
    gl_InstanceIndex = stage_input.gl_InstanceIndex;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
