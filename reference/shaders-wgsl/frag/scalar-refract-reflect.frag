var<private> FragColor : f32;
var<private> vRefract : vec3f;

fn frag_main()
{
    FragColor = refract(vec2f(vRefract.x, 0.0f), vec2f(vRefract.y, 0.0f), vRefract.z).x;
    FragColor += reflect(vec2f(vRefract.x, 0.0f), vec2f(vRefract.y, 0.0f)).x;
    FragColor += refract(vRefract.xy, vRefract.yz, vRefract.z).y;
    FragColor += reflect(vRefract.xy, vRefract.zy).y;
}

struct SPIRV_Cross_Input
{
    @location(0) vRefract : vec3f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : f32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vRefract = stage_input.vRefract;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
