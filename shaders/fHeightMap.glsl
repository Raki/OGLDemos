#version 460


out vec4 fragColor;

in float height;

subroutine vec3 GetColorType();
subroutine uniform GetColorType GetColor;


subroutine(GetColorType)
vec3 getSingleColor()
{
	float c = (height+16.)/32.;
	vec3 color = vec3(c,c,c);
	return color;
}

subroutine(GetColorType)
vec3 getMultiColor()
{
	vec3 c1=vec3(237./255.,28./255.,36./255.);
	vec3 c2=vec3(255./255.,127./255.,39./255.);
	vec3 c3=vec3(237./255.,28./255.,36./255.);
	float c = (height+16.)/32.;
	vec3 color = vec3(c,c,c);
	if(c>=0.8)
	{
		float fact = 1./(1.-0.8);
		float f = (0.8-c)*fact;
		color = (1.-f)*c2+f*c1;
	}
	else if(c>=0.6&&c<0.8)
	{
		float fact = 1./(0.8-0.6);
		float f = (0.6-c)*fact;
		color = (1.-f)*c3+f*c2;
	}
	else if(c>=0.4&&c<0.6)
	{
		float fact = 1./(0.6-0.4);
		float f = (0.4-c)*fact;
		color = (1.-f)*color+f*c3;
	}
	return color;
}

void main()
{
	vec3 color = GetColor();
	fragColor = vec4(color,1);
}