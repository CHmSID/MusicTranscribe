#version 330

in float isWhite;
out vec4 fColor;

void main(){

	vec3 colorWhite = vec3(252/255f, 109/255f, 38/255f);
	vec3 colorBlack = vec3(226/255f, 67/255f, 41/255f);

	if(isWhite == 1)
		fColor = vec4(colorWhite, 1);
	else
		fColor = vec4(colorBlack, 1);
}
