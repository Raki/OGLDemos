#version 460


out vec4 fragColor;
in vec3 uv_out;

uniform samplerCube sampler;
void main()
{
	fragColor = texture(sampler,uv_out);
}