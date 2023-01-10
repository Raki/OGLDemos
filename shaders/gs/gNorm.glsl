#version 460

layout (triangles) in;
layout (line_strip, max_vertices=6) out;

layout (location=0) in vec3 gs_normal[];

layout (location=0) out vec3 fs_normal;

uniform mat4 proj;

void main()
{
	for(int i=0;i<3;i++)
	{
		gl_Position = proj*(gl_in[i].gl_Position);
		EmitVertex();
		gl_Position = proj*(gl_in[i].gl_Position+(vec4(gs_normal[i],0.)*2.));
		EmitVertex();
		EndPrimitive();
	}

}