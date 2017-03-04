#version 330

layout(location = 0) in vec2 vertexPosition;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

void main()
{
	gl_Position = projectionMatrix * modelMatrix *
	        vec4(vertexPosition, 0.0f, 1.0f);
}
