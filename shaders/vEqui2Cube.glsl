#version 460

uniform mat4 view;
uniform mat4 proj;


layout (location=0) in vec3 position;

out vec3 localPos;

void main()
{
	localPos = position;
	gl_Position = proj*view*vec4(position,1.0);
}