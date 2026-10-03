struct Push
{
    mvp : mat4x4f,
    tint : vec4f,
}

struct Material
{
    color : vec4f,
    roughness : f32,
}

struct VertexOut
{
    normal : vec3f,
    bone : u32,
}

@group(0) @binding(0) var<uniform> push : Push;

var<private> gl_Position : vec4f;
var<private> aInstanceMatrix : mat4x4f;
var<private> aPosition : vec4f;
var<private> vColor : vec4f;
var<private> gl_VertexIndex : i32;
var<private> gl_InstanceIndex : i32;
var<private> vIndex : i32;
var<private> aIndices : vec2i;
var<private> vUVs : array<vec2f, 2>;
var<private> aUVs : array<vec2f, 2>;
var<private> vLinear : f32;
var<private> vCentroid : vec3f;
var<private> vMaterial : Material;
var<private> vOut : VertexOut;
var<private> aBones : vec4u;

fn vert_main()
{
    gl_Position = (push.mvp * aInstanceMatrix) * aPosition;
    vColor = (push.tint * f32(gl_VertexIndex)) + vec4f(f32(gl_InstanceIndex));
    vIndex = aIndices.x + aIndices.y;
    vUVs = aUVs;
    vLinear = aPosition.w;
    vCentroid = aPosition.xyz;
    vMaterial.color = push.tint;
    vMaterial.roughness = 0.5f;
    vOut.normal = aInstanceMatrix[2].xyz;
    vOut.bone = aBones.x + aBones.w;
}

struct SPIRV_Cross_Input
{
    @location(0) aPosition : vec4f,
    @location(1) aInstanceMatrix_0 : vec4f,
    @location(2) aInstanceMatrix_1 : vec4f,
    @location(3) aInstanceMatrix_2 : vec4f,
    @location(4) aInstanceMatrix_3 : vec4f,
    @location(5) aIndices : vec2i,
    @location(6) aBones : vec4u,
    @location(7) aUVs_0 : vec2f,
    @location(8) aUVs_1 : vec2f,
    @builtin(vertex_index) gl_VertexIndex : u32,
    @builtin(instance_index) gl_InstanceIndex : u32,
}

struct SPIRV_Cross_Output
{
    @location(0) vColor : vec4f,
    @location(1) @interpolate(flat) vIndex : i32,
    @location(2) vUVs_0 : vec2f,
    @location(3) vUVs_1 : vec2f,
    @location(4) @interpolate(linear) vLinear : f32,
    @location(5) @interpolate(perspective, centroid) vCentroid : vec3f,
    @location(6) vMaterial_color : vec4f,
    @location(7) vMaterial_roughness : f32,
    @location(8) vOut_normal : vec3f,
    @location(9) @interpolate(flat) vOut_bone : u32,
    @builtin(position) gl_Position : vec4f,
}

@vertex
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    aPosition = stage_input.aPosition;
    aInstanceMatrix[0] = stage_input.aInstanceMatrix_0;
    aInstanceMatrix[1] = stage_input.aInstanceMatrix_1;
    aInstanceMatrix[2] = stage_input.aInstanceMatrix_2;
    aInstanceMatrix[3] = stage_input.aInstanceMatrix_3;
    aIndices = stage_input.aIndices;
    aBones = stage_input.aBones;
    aUVs[0] = stage_input.aUVs_0;
    aUVs[1] = stage_input.aUVs_1;
    gl_VertexIndex = i32(stage_input.gl_VertexIndex);
    gl_InstanceIndex = i32(stage_input.gl_InstanceIndex);
    vert_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.vColor = vColor;
    stage_output.vIndex = vIndex;
    stage_output.vUVs_0 = vUVs[0];
    stage_output.vUVs_1 = vUVs[1];
    stage_output.vLinear = vLinear;
    stage_output.vCentroid = vCentroid;
    stage_output.vMaterial_color = vMaterial.color;
    stage_output.vMaterial_roughness = vMaterial.roughness;
    stage_output.vOut_normal = vOut.normal;
    stage_output.vOut_bone = vOut.bone;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
