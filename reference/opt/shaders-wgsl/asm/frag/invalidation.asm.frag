var<private> v0 : f32;
var<private> v1 : f32;
var<private> FragColor : f32;

fn frag_main()
{
    FragColor = (v0 + v1) * v1;
}

struct SPIRV_Cross_Input
{
    @location(0) v0 : f32,
    @location(1) v1 : f32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    v0 = stage_input.v0;
    v1 = stage_input.v1;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
