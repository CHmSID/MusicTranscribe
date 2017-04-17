#version 330

in float isWhite;
out vec4 fColor;

void main(){

	vec3 colorWhite = vec3(252.0/255.0, 109.0/255.0, 38.0/255.0);
	vec3 colorBlack = vec3(226.0/255.0, 67.0/255.0, 41.0/255.0);

	if(isWhite == 1)
		fColor = vec4(colorWhite, 1);
	else
		fColor = vec4(colorBlack, 1);
}
