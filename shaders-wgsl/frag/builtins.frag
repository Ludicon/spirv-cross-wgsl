#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 c = gl_FragCoord;
	if (gl_FrontFacing)
		c *= 2.0;
	c += float(gl_SampleID);
	gl_SampleMask[0] = gl_SampleMaskIn[0] & 3;
	gl_FragDepth = c.z * 0.5;
	FragColor = c;
}
