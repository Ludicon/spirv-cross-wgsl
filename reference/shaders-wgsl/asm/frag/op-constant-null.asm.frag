struct D
{
    a : vec4f,
    b : f32,
}

const _41 : array<vec4f, 4> = array<vec4f, 4>(vec4f(0.0f), vec4f(0.0f), vec4f(0.0f), vec4f(0.0f));

var<private> FragColor : f32;

fn frag_main()
{
    let a = 0.0f;
    let b = vec4f(0.0f);
    let c = mat2x3f(vec3f(0.0f), vec3f(0.0f));
    let d = D(vec4f(0.0f), 0.0f);
    FragColor = a;
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
