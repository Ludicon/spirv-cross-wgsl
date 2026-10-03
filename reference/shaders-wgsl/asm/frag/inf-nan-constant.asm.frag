fn spvNonFinite(bits : u32) -> f32
{
    return bitcast<f32>(bits);
}

var<private> FragColor : vec3f;

fn frag_main()
{
    FragColor = vec3f(spvNonFinite(0x7f800000u /* inf */), spvNonFinite(0xff800000u /* -inf */), spvNonFinite(0x7fc00000u /* nan */));
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec3f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
