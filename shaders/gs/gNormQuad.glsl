#version 460

layout (triangles) in;
layout (line_strip, max_vertices=6) out;

layout (location=0) in vec3 gs_normal[];

layout (location=0) out vec3 fs_normal;

uniform mat4 proj;

void main()
{
	for(int i=0;i<1;i++)
	{
		int j = (i==2)?0:i+1;
		gl_Position = proj*(gl_in[i].gl_Position);
		gl_Position.z-=0.01;
		EmitVertex();
		gl_Position = proj*(gl_in[j].gl_Position);
		gl_Position.z-=0.01;
		EmitVertex();
		EndPrimitive();
	}

}