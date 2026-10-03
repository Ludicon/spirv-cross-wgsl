enable clip_distances;

var<private> gl_Position : vec4f;
var<private> gl_PointSize : f32;
var<private> gl_ClipDistance : array<f32, 2>;
var<private> aPosition : vec4f;

fn vert_main()
{
    gl_Position = aPosition;
    gl_PointSize = 1.0f;
    gl_ClipDistance[0] = aPosition.x;
    gl_ClipDistance[1] = aPosition.y;
}

struct SPIRV_Cross_Input
{
    @location(0) aPosition : vec4f,
}

struct SPIRV_Cross_Output
{
    @builtin(position) @invariant gl_Position : vec4f,
    @builtin(clip_distances) gl_ClipDistance : array<f32, 2>,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    aPosition = stage_input.aPosition;
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.gl_Position = gl_Position;
    stage_output.gl_ClipDistance = gl_ClipDistance;
    return stage_output;
}
