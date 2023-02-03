#version 460


out vec4 fragColor;

uniform vec3 lightPos;
uniform vec3 lightColr;

uniform vec3 color;
uniform float lReach;

in vec3 fragPos;
in vec3 nrml_out;
#ifdef PER_VERTEX_COLOR
in vec3 color_out;
#else
in vec2 uv_out;
#endif

subroutine vec3 GetColorType();
subroutine uniform GetColorType GetColor;


float getDiffuseFactr(vec3 lPos)
{
	vec3 lightDir = normalize(lPos-fragPos); 
	float diffuse = max(dot(nrml_out,lightDir),0.);

	return diffuse;
}

subroutine(GetColorType)
vec3 getDiffuseColrVertex()
{
	#ifdef PER_VERTEX_COLOR
	vec3 res0 = lightColr*color_out*getDiffuseFactr(glm::vec3(lightPos.x-lReach,lightPos.y,lightPos.z+lReach));
	vec3 res1 = lightColr*color_out*getDiffuseFactr(glm::vec3(lightPos.x+lReach,lightPos.y,lightPos.z+lReach));
	vec3 res2 = lightColr*color_out*getDiffuseFactr(glm::vec3(lightPos.x-lReach,lightPos.y,lightPos.z-lReach));
	vec3 res3 = lightColr*color_out*getDiffuseFactr(glm::vec3(lightPos.x+lReach,lightPos.y,lightPos.z-lReach));
	return res0+res1+res2+res3;
	#else
	return lightColr*color;
	#endif
}

subroutine(GetColorType)
vec3 getDiffuseColr()
{
	return lightColr*color*getDiffuseFactr(lightPos);
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