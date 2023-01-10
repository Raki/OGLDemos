#version 460

uniform mat4 model;
uniform mat4 view;
uniform mat4 nrmlMat;

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

layout (location=0) out vec3 gs_normal;



void main()
{
	gs_normal = mat3(nrmlMat)*normal;
	gl_Position = view*model*vec4(position,1.0);
}