#version 450

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 FragColor;

struct Light
{
	vec3 dir;
	float intensity;
};

void modify(inout vec4 v, out float f, in float scale)
{
	v *= scale;
	f = v.x + v.y;
}

void modify_struct(inout Light l)
{
	l.intensity *= 2.0;
	l.dir = normalize(l.dir);
}

float overload(float x) { return x * 2.0; }
vec2 overload(vec2 x) { return x * 3.0; }

vec4 g_color;

void write_global()
{
	g_color = vColor * 0.5;
}

float lights[4] = float[](1.0, 2.0, 3.0, 4.0);

float sum(float arr[4])
{
	float s = 0.0;
	for (int i = 0; i < 4; i++)
		s += arr[i];
	return s;
}

void main()
{
	vec4 c = vColor;
	float f;
	modify(c, f, 2.0);
	Light l = Light(vec3(1.0, 2.0, 3.0), 1.0);
	modify_struct(l);
	write_global();
	FragColor = c + vec4(l.dir * l.intensity, f) + g_color + vec4(overload(f), overload(vec2(f)), sum(lights));
}
