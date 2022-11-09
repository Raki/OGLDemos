#version 460

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 nrmlMat;

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uvcoord;


out vec3 normal_out;
out vec2 uv_out;
out vec3 fragPos;

void main()
{

	normal_out =mat3(nrmlMat)*normal;
	uv_out = uvcoord;
	gl_Position = proj*view*model*vec4(position,1.0);
	fragPos = vec3(model*vec4(position,1.0));

}