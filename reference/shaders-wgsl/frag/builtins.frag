var<private> gl_FragCoord : vec4f;
var<private> gl_FrontFacing : bool;
var<private> gl_SampleID : i32;
var<private> gl_SampleMask : array<i32, 1>;
var<private> gl_SampleMaskIn : array<i32, 1>;
var<private> gl_FragDepth : f32;
var<private> FragColor : vec4f;

fn frag_main()
{
    var c = gl_FragCoord;
    if (gl_FrontFacing)
    {
        c *= 2.0f;
    }
    c += vec4f(f32(gl_SampleID));
    gl_SampleMask[0] = gl_SampleMaskIn[0] & 3;
    gl_FragDepth = c.z * 0.5f;
    FragColor = c;
}

struct SPIRV_Cross_Input
{
    @builtin(position) gl_FragCoord : vec4f,
    @builtin(front_facing) gl_FrontFacing : bool,
    @builtin(sample_index) gl_SampleID : u32,
    @builtin(sample_mask) gl_SampleMaskIn : u32,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
    @builtin(sample_mask) gl_SampleMask : u32,
    @builtin(frag_depth) gl_FragDepth : f32,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FrontFacing = stage_input.gl_FrontFacing;
    gl_SampleID = i32(stage_input.gl_SampleID);
    gl_SampleMaskIn[0] = i32(stage_input.gl_SampleMaskIn);
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    stage_output.gl_SampleMask = u32(gl_SampleMask[0]);
    stage_output.gl_FragDepth = gl_FragDepth;
    return stage_output;
}
