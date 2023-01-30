#version 460

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 nrmlMat;


layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uv;

out vec3 fragPos;
out vec3 nrml_out;
out vec2 uv_out;

void main()
{
	uv_out = uv;
	nrml_out = vec3(nrmlMat*vec4(normal,0.));
	fragPos = vec3(model*vec4(position,1.0));
	gl_Position = proj*view*model*vec4(position,1.0);
}