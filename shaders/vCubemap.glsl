#version 460

uniform mat4 view;
uniform mat4 proj;

layout (location=0) in vec3 position;

out vec3 uv_out;

void main()
{
	uv_out = normalize(position);
	vec4 pos = proj*view*vec4(position,1.0);
	gl_Position = vec4(pos.x,pos.y,pos.z,pos.z);
}