#version 330 core

in vec2 UV;

out vec4 fColor;

uniform sampler2D tex;

void main()
{
	    fColor = texture(tex, UV);
}
