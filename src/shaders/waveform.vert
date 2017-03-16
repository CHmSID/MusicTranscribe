#version 330
layout(location = 0) in vec2 vertex;
layout(location = 1) in vec3 color;

uniform mat4 projectionMatrix;

out vec3 vColor;

void main()
{
	vColor = color;
	gl_Position = projectionMatrix * vec4(vertex, 0.0f, 1.0f);
}
