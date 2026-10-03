enable dual_source_blending;

var<private> FragColor0 : vec4f;
var<private> FragColor1 : vec4f;

fn frag_main()
{
    FragColor0 = vec4f(1.0f);
    FragColor1 = vec4f(2.0f);
}

struct SPIRV_Cross_Output
{
    @location(0) @blend_src(0) FragColor0 : vec4f,
    @location(0) @blend_src(1) FragColor1 : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor0 = FragColor0;
    stage_output.FragColor1 = FragColor1;
    return stage_output;
}
