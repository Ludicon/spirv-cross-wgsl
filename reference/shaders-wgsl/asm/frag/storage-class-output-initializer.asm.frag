var<private> FragColors : array<vec4f, 2>;
var<private> FragColor : vec4f;

fn frag_main()
{
}

struct SPIRV_Cross_Output
{
    @location(0) FragColors_0 : vec4f,
    @location(1) FragColors_1 : vec4f,
    @location(2) FragColor : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColors_0 = FragColors[0];
    stage_output.FragColors_1 = FragColors[1];
    stage_output.FragColor = FragColor;
    return stage_output;
}
