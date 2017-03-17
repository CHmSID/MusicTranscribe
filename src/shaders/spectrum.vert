#version 330
layout(location = 0) in vec2 vertex;
layout(location = 1) in float white;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

out float isWhite;

void main(){

	gl_Position = projectionMatrix * modelMatrix *
	        vec4(vertex, 0.0f, 1.0f);

	isWhite = white;
}
