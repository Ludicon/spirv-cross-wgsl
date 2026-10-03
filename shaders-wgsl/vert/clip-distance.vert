#version 450

layout(location = 0) in vec4 aPosition;

out gl_PerVertex
{
	invariant vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[2];
};

void main()
{
	gl_Position = aPosition;
	gl_PointSize = 1.0;
	gl_ClipDistance[0] = aPosition.x;
	gl_ClipDistance[1] = aPosition.y;
}
