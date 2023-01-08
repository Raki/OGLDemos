#version 460

layout (points) in;
layout (triangle_strip, max_vertices=3) out;

uniform float tLen;

void emitTriangle(vec4 pos)
{
	gl_Position = pos + vec4(-tLen,-tLen,0,0);
	EmitVertex();
	gl_Position = pos + vec4(-tLen,tLen,0,0);
	EmitVertex();
	gl_Position = pos + vec4(tLen,tLen,0,0);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	emitTriangle(gl_in[0].gl_Position);
}