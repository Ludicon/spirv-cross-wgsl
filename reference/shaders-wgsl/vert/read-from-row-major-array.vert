struct Block
{
    _var : array<array<mat3x4f, 4>, 3>,
}

@group(0) @binding(0) var<uniform> _104 : Block;

var<private> gl_Position : vec4f;
var<private> a_position : vec4f;
var<private> v_vtxResult : f32;

fn compare_float(a : f32, b : f32) -> f32
{
    return select(0.0f, 1.0f, abs(a - b) < 0.0500000007450580596923828125f);
}

fn compare_vec3(a : vec3f, b : vec3f) -> f32
{
    let param = a.x;
    let param_1 = b.x;
    let param_2 = a.y;
    let param_3 = b.y;
    let param_4 = a.z;
    let param_5 = b.z;
    return (compare_float(param, param_1) * compare_float(param_2, param_3)) * compare_float(param_4, param_5);
}

fn compare_mat2x3(a : mat2x3f, b : mat2x3f) -> f32
{
    let param = a[0];
    let param_1 = b[0];
    let param_2 = a[1];
    let param_3 = b[1];
    return compare_vec3(param, param_1) * compare_vec3(param_2, param_3);
}

fn vert_main()
{
    gl_Position = a_position;
    var result = 1.0f;
    let param = transpose(mat3x2f(_104._var[0][0][0].xy, _104._var[0][0][1].xy, _104._var[0][0][2].xy));
    let param_1 = mat2x3f(vec3f(2.0f, 6.0f, -6.0f), vec3f(0.0f, 5.0f, 5.0f));
    result *= compare_mat2x3(param, param_1);
    v_vtxResult = result;
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
