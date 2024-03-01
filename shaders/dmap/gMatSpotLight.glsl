#version 460

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

//layout (location=0) in vec3 gs_normal[];
//layout (location=1) in vec3 gs_fragPos[];
//layout (location=2) in vec2 gs_uv[];

in VS_OUT
{
	vec3 normal;
	vec3 fragPos;
	vec2 uv;
}gs_in[];

out GS_OUT
{
	vec3 normal;
	vec3 fragPos;
	vec2 uv;
}gs_out;

//layout (location=0) out vec3 fs_normal;
//layout (location=1) out vec3 fs_fragPos;
//layout (location=2) out vec2 fs_uv;

uniform float nDist;

vec3 getNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}  

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main()
{
	vec3 norm = getNormal();
	gl_Position = gl_in[0].gl_Position;
	gs_out.normal = norm;
	gs_out.uv= gs_in[0].uv;
	gs_out.fragPos= gs_in[0].fragPos;
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	gs_out.normal = norm;
	gs_out.uv= gs_in[1].uv;
	gs_out.fragPos= gs_in[1].fragPos;
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	gs_out.normal = norm;
	gs_out.uv= gs_in[2].uv;
	gs_out.fragPos= gs_in[2].fragPos;
	EmitVertex();

	EndPrimitive();
}