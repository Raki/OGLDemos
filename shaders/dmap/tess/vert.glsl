#version 460

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 proj;
//uniform mat4 nrmlMat;

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uvcoord;

//layout (location=0) out vec3 gs_normal;
//layout (location=1) out vec3 gs_fragPos;
//layout (location=2) out vec2 gs_uv;

out VS_OUT
{
	vec3 normal;
	vec2 uv;
}vs_out;


void main()
{
	//vs_out.normal = mat3(nrmlMat)*normal;
	vs_out.normal = normal;
	vs_out.uv = uvcoord;
	//gl_Position = proj*view*model*vec4(position,1.0);
	gl_Position = vec4(position,1.0);
	//vs_out.fragPos = vec3(model*vec4(position,1.0));
}