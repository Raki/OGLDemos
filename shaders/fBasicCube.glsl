#version 460


out vec4 fragColor;
in vec3 o_color;


void main()
{
	fragColor = vec4(o_color,1.0);
}