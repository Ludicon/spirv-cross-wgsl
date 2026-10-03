struct RowMajor
{
    B : mat4x4f,
}

struct NestedRowMajor
{
    rm : RowMajor,
}

struct RowMajor_1
{
    B : mat4x4f,
}

struct NestedRowMajor_1
{
    rm : RowMajor_1,
}

struct UBO3
{
    rm2 : NestedRowMajor_1,
}

struct UBO2
{
    rm : RowMajor_1,
}

struct UBO
{
    A : mat4x4f,
    C : mat4x4f,
}

struct UBONoWorkaround
{
    D : mat4x4f,
}

@group(0) @binding(2) var<uniform> _17 : UBO3;
@group(0) @binding(1) var<uniform> _35 : UBO2;
@group(0) @binding(0) var<uniform> _42 : UBO;
@group(0) @binding(3) var<uniform> _56 : UBONoWorkaround;

var<private> FragColor : vec4f;
var<private> Clip : vec4f;

fn frag_main()
{
    var rm2_loaded : NestedRowMajor;
    rm2_loaded.rm.B = transpose(_17.rm2.rm.B);
    FragColor = (((rm2_loaded.rm.B * transpose(_35.rm.B)) * transpose(_42.A)) * _42.C) * Clip;
    FragColor += (_56.D * Clip);
    FragColor += (vec4f(_42.A[0][1], _42.A[1][1], _42.A[2][1], _42.A[3][1]) * Clip);
}

struct SPIRV_Cross_Input
{
    @location(0) Clip : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    Clip = stage_input.Clip;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
