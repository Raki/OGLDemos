#version 460

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 nrmlMat;
uniform float depth;
uniform sampler2D dispMap;

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uvcoord;


out VS_OUT
{
	vec3 normal;
	vec3 fragPos;
	vec2 uv;
}vs_out;

//out vec3 normal_out;
//out vec2 uv_out;
//out vec3 fragPos;

void main()
{

	
	vs_out.uv = uvcoord;
	vec2 normPos = vec2((position.x+2.0)/4.0,(position.y+2.0)/4.0);
	float r = texture(dispMap,normPos).r*depth;
	vec3 modifiedPos = position+vec3(0,0,-r);
	vs_out.normal = normal;
	gl_Position = proj*view*model*vec4(modifiedPos,1.0);
	vs_out.fragPos = vec3(model*vec4(position,1.0));

}