#version 460
#extension GL_KHR_vulkan_glsl : enable

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


layout (std140,binding = 1) uniform Instances
{
	vec3 iPositions[];
};


void main()
{
	uint vx = gl_VertexID;
    uint instance = vx >> 3;
	
	uvec3 xyz = uvec3(vx & 0x1, (vx & 0x4) >> 2, (vx & 0x2) >> 1);
	vec3 uvw = vec3(xyz);
    vec3 position = uvw * 2.0 - 1.0;

	gl_Position = proj*view*model*vec4(position,1.0);
}