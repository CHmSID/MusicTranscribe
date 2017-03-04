#version 330

out vec4 outColor;
uniform vec3 currentColor;

void main()
{
	outColor = vec4(currentColor, 1.0f);
}
