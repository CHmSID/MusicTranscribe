#version 330

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec3 color;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec3 vColor;

void main()
{
	gl_Position = projectionMatrix * modelMatrix *
	        vec4(vertex, 0.0f, 1.0f);

	vColor = color;
}

