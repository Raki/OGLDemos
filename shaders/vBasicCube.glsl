#version 460
#extension GL_KHR_vulkan_glsl : enable

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 camera_pos;

struct InstanceData
{
	vec4 pos;
	vec4 color;
};

layout (std430,binding = 1) readonly buffer Instances
{
	InstanceData instances[];
};

out vec3 o_color;

void main()
{  
	uint vx = gl_VertexID;
    uint instance = vx >> 3;
	
	vec3 instance_pos = instances[instance].pos.xyz;
    vec3 local_camera_pos = camera_pos.xyz - instance_pos;

	uvec3 xyz = uvec3(vx & 0x1, (vx & 0x4) >> 2, (vx & 0x2) >> 1);

	if (local_camera_pos.x > 0) xyz.x = 1 - xyz.x;
    if (local_camera_pos.y > 0) xyz.y = 1 - xyz.y;
    if (local_camera_pos.z > 0) xyz.z = 1 - xyz.z;

	vec3 uvw = vec3(xyz);
    vec3 position = uvw * 2.0 - 1.0;

	o_color = uvw;//instances[instance].color.xyz;
	gl_Position = proj*view*model*vec4(position+instance_pos,1.0);
}