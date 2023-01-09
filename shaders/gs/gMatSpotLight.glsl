#version 460

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

layout (location=0) in vec3 gs_normal[];
layout (location=1) in vec3 gs_fragPos[];
layout (location=2) in vec2 gs_uv[];


layout (location=0) out vec3 fs_normal;
layout (location=1) out vec3 fs_fragPos;
layout (location=2) out vec2 fs_uv;

uniform float nDist;

vec3 getNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}  

void main()
{
	vec3 norm = getNormal();
	gl_Position = gl_in[0].gl_Position+glm::vec4((nDist*norm),0);
	fs_normal = gs_normal[0];
	fs_uv= gs_uv[0];
	fs_fragPos= gs_fragPos[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position+glm::vec4((nDist*norm),0);
	fs_normal = gs_normal[1];
	fs_uv= gs_uv[1];
	fs_fragPos= gs_fragPos[1];
	EmitVertex();

	gl_Position = gl_in[2].gl_Position+glm::vec4((nDist*norm),0);
	fs_normal = gs_normal[2];
	fs_uv= gs_uv[2];
	fs_fragPos= gs_fragPos[2];
	EmitVertex();

	EndPrimitive();
}