#version 460


out vec4 fragColor;

uniform vec3 lightPos;
uniform vec3 lightColr;

uniform vec3 color;

in vec3 fragPos;
in vec3 nrml_out;
in vec2 uv_out;

subroutine vec3 GetColorType();
subroutine uniform GetColorType GetColor;


float getDiffuseFactr()
{
	vec3 lightDir = normalize(lightPos-fragPos); 
	float diffuse = max(dot(nrml_out,lightDir),0.);

	return diffuse;
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