#version 460


struct Material
{
	sampler2D diffuseSampler;
	sampler2D specularSampler;

	float shininess;
};

struct Light {
    vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

out vec4 fragColor;

in GS_OUT
{
	vec3 normal;
	vec3 fragPos;
	vec2 uv;
}fs_in;

//layout (location=0) in vec3 fs_normal;
//layout (location=1) in vec3 fs_fragPos;
//layout (location=2) in vec2 fs_uv;
//

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
	vec3 normal_out = fs_in.normal;
	vec3 fragPos = fs_in.fragPos;
	vec2 uv_out = fs_in.uv;

	vec3 result;
	vec3 lightDir = normalize(light.position - fragPos);  
	float theta = dot(lightDir,-light.direction);
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);   

	//if(theta>light.cutOff)
	//{
		

		//attenuation
		float dist = length(light.position-fragPos); 
		float attenuation = 1.0/(light.constant+ light.linear*dist + light.quadratic*(dist*dist));

		//ambient
		vec3 ambient = light.ambient* vec3(texture(material.diffuseSampler,uv_out));
		ambient*=attenuation;

		//diffuse
		vec3 norm = normalize(normal_out);
		float diff = max(dot(norm,lightDir),0.0);
		vec3 diffuse = light.diffuse*(diff*vec3(texture(material.diffuseSampler,uv_out)));
		diffuse*=attenuation;
		diffuse*=intensity;


		//specular
		vec3 viewDir = normalize(viewPos-fragPos);
		vec3 reflectDir = reflect(-lightDir,norm);
		float spec = pow(max(dot(viewDir,reflectDir),0),material.shininess);
		vec3 specular = light.specular*(spec*vec3(texture(material.specularSampler,uv_out)));
		specular*=attenuation;
		specular*=intensity;

		result = ambient+diffuse+specular;
	//}
	//else
	//{
	//	result = light.ambient*vec3(texture(material.diffuseSampler,uv_out));
	//}
	fragColor = vec4(result,1.0);
}