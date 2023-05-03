#version 460

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 nrmlMat;


layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
#ifdef PER_VERTEX_COLOR
layout (location=2) in vec3 vColor;
out vec3 color_out;
#else
layout (location=2) in vec2 uv;
out vec2 TexCoords;
#endif

out vec3 WorldPos;
out vec3 Normal;



void main()
{
#ifdef PER_VERTEX_COLOR
	color_out = vColor;
#else
	TexCoords  = uv;
#endif
	Normal = vec3(nrmlMat*vec4(normal,0.));
	WorldPos = vec3(model*vec4(position,1.0));
	gl_Position = proj*view*model*vec4(position,1.0);
}