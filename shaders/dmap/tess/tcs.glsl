#version 460

layout (vertices=4) out;


in VS_OUT
{
	vec3 normal;
	vec2 uv;
}tcs_in[];

out VS_OUT
{
	vec3 normal;
	vec2 uv;
}tcs_out[];


uniform int tessLevel;

void main()
{
	// pass attributes through
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tcs_out[gl_InvocationID].normal = tcs_in[gl_InvocationID].normal;
	tcs_out[gl_InvocationID].uv = tcs_in[gl_InvocationID].uv;

	// invocation zero controls tessellation levels for the entire patch
	if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;
							   
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}