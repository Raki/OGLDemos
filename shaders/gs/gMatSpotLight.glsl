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

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main()
{
	vec3 norm = getNormal();
	float rnd = random( gs_uv[0] );
	gl_Position = gl_in[0].gl_Position+vec4((((sin(nDist) + 1.0) / 2.0) *norm*2.*rnd),0);
	fs_normal = gs_normal[0];
	fs_uv= gs_uv[0];
	fs_fragPos= gs_fragPos[0];
	EmitVertex();

	rnd = random( gs_uv[1] );
	gl_Position = gl_in[1].gl_Position+vec4((((sin(nDist) + 1.0) / 2.0) *norm*2.*rnd),0);
	fs_normal = gs_normal[1];
	fs_uv= gs_uv[1];
	fs_fragPos= gs_fragPos[1];
	EmitVertex();

	rnd = random( gs_uv[2] );
	gl_Position = gl_in[2].gl_Position+vec4((((sin(nDist) + 1.0) / 2.0) *norm*2.*rnd),0);
	fs_normal = gs_normal[2];
	fs_uv= gs_uv[2];
	fs_fragPos= gs_fragPos[2];
	EmitVertex();

	EndPrimitive();
}