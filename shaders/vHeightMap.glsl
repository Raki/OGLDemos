#version 460

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


layout (location=0) in vec3 position;

out float height;

void main()
{
	height= position.y;
	gl_Position = proj*view*model*vec4(position,1.0);
}