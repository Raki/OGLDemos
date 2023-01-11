#version 460


out vec4 fragColor;

in float height;

void main()
{
	float c = (height+16.)/32.;
	fragColor = vec4(c,c,c,1.0);
}