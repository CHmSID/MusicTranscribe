#version 330

in vec3 vColor;

layout(location = 0) out vec4 fColor;

void main()
{
	fColor = vec4(vColor, 1.0f);
}
