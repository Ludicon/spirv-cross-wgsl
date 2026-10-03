struct Block
{
    _var : array<array<mat3x4f, 4>, 3>,
}

@group(0) @binding(0) var<uniform> _104 : Block;

var<private> gl_Position : vec4f;
var<private> a_position : vec4f;
var<private> v_vtxResult : f32;

fn vert_main()
{
    gl_Position = a_position;
    v_vtxResult = ((select(0.0f, 1.0f, abs(_104._var[0][0][0][0] - 2.0f) < 0.0500000007450580596923828125f) * select(0.0f, 1.0f, abs(_104._var[0][0][1][0] - 6.0f) < 0.0500000007450580596923828125f)) * select(0.0f, 1.0f, abs(_104._var[0][0][2][0] - (-6.0f)) < 0.0500000007450580596923828125f)) * ((select(0.0f, 1.0f, abs(_104._var[0][0][0][1]) < 0.0500000007450580596923828125f) * select(0.0f, 1.0f, abs(_104._var[0][0][1][1] - 5.0f) < 0.0500000007450580596923828125f)) * select(0.0f, 1.0f, abs(_104._var[0][0][2][1] - 5.0f) < 0.0500000007450580596923828125f));
}

struct SPIRV_Cross_Input
{
    @location(0) a_position : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) v_vtxResult : f32,
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    a_position = stage_input.a_position;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.v_vtxResult = v_vtxResult;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
