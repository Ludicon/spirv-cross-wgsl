struct UBO
{
    mvp : mat4x4f,
}

@group(0) @binding(0) var<uniform> _16 : UBO;

var<private> gl_Position : vec4f;
var<private> aVertex : vec4f;
var<private> vNormal : vec3f;
var<private> aNormal : vec3f;

fn vert_main()
{
    gl_Position = _16.mvp * aVertex;
    vNormal = aNormal;
}

struct SPIRV_Cross_Input
{
    @location(0) aVertex : vec4f,
    @location(1) aNormal : vec3f,
}

struct SPIRV_Cross_Output
{
    @location(0) vNormal : vec3f,
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    aVertex = stage_input.aVertex;
    aNormal = stage_input.aNormal;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.vNormal = vNormal;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
