#version 450

layout(location = 0) flat in int vMode;
layout(location = 1) in vec4 vColor;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform UBO
{
	int count;
	int limit;
};

void main()
{
	vec4 c = vColor;

	switch (vMode)
	{
	case 0:
		c *= 2.0;
		break;
	case 1:
	case 2:
		c += 1.0;
		// fallthrough
	case 3:
		c = c.wzyx;
		break;
	case -1:
		discard;
	default:
		c = vec4(0.0);
		break;
	}

	int i = 0;
	do
	{
		c.x += 0.1;
		i++;
	} while (i < count);

	for (int j = 0, k = 10; j < k; j++, k--)
		c.y += float(j * k);

	for (int j = 0; j < count; j++)
	{
		if (j == limit)
			break;
		if ((j & 1) != 0)
			continue;
		for (int k = 0; k < count; k++)
		{
			switch (k)
			{
			case 2:
				break;
			case 5:
				c.z += 1.0;
				continue;
			default:
				c.w -= 0.5;
				break;
			}
			if (c.w < -10.0)
				break;
		}
	}

	while (c.x < 100.0)
		c.x *= 2.0;

	FragColor = c.x > 50.0 ? c : c.yxwz;
}
