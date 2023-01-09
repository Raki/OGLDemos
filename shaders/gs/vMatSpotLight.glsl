#version 460

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 nrmlMat;

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uvcoord;

layout (location=0) out vec3 gs_normal;
layout (location=1) out vec3 gs_fragPos;
layout (location=2) out vec2 gs_uv;



void main()
{
	gs_normal = mat3(nrmlMat)*normal;
	gs_uv = uvcoord;
	gl_Position = proj*view*model*vec4(position,1.0);
	gs_fragPos = vec3(model*vec4(position,1.0));
}