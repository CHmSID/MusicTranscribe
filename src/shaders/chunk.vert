#version 330 core
layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 uv;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec2 UV;

void main()
{
	UV = uv;

	gl_Position = projectionMatrix * modelMatrix *
	        vec4(vertex, 0.0f, 1.0f);
}
