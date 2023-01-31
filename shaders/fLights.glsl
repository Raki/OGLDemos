#version 460


out vec4 fragColor;

uniform vec3 lightPos;
uniform vec3 lightColr;

uniform vec3 color;

in vec3 fragPos;
in vec3 nrml_out;
#ifdef PER_VERTEX_COLOR
in vec3 color_out;
#else
in vec2 uv_out;
#endif

subroutine vec3 GetColorType();
subroutine uniform GetColorType GetColor;


float getDiffuseFactr()
{
	vec3 lightDir = normalize(lightPos-fragPos); 
	float diffuse = max(dot(nrml_out,lightDir),0.);

	return diffuse;
}

subroutine(GetColorType)
vec3 getDiffuseColrVertex()
{
	#ifdef PER_VERTEX_COLOR
	return lightColr*color_out*getDiffuseFactr();
	#else
	return lightColr*color;
	#endif
}

subroutine(GetColorType)
vec3 getDiffuseColr()
{
	return lightColr*color*getDiffuseFactr();
}

subroutine(GetColorType)
vec3 getColr()
{
	return lightColr*color;
}

void main()
{
	vec3 finalColor = GetColor();
	fragColor = vec4(finalColor,1.0);
}