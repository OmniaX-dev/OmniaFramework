#version 410 core

out vec4 FragColor;

in vec2 outTexCoords;
in vec3 outNormal;
in vec3 outFragPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct LightSource {
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

	int type;
};

const int SpotLight = 0;
const int Directional = 1;
const int PointLight = 2;

//@[skip_auto_register]
uniform Material material;
//@[skip_auto_register]
uniform LightSource light;

uniform vec3 uViewPosition;

//float near = 0.1; 
//float far  = 100.0; 
  
//float LinearizeDepth(float depth) 
//{
//    float z = depth * 2.0 - 1.0; // back to NDC 
//    return (2.0 * near * far) / (far + near - z * (far - near));	
//}

void main()
{
	//Ambient light
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, outTexCoords));
	vec3 norm = normalize(outNormal);
	vec3 lightDir;
	float intensity = 1.0;
	if (light.type == PointLight)
	{
		lightDir = normalize(light.position - outFragPos);
	}
	else if (light.type == Directional)
	{
		lightDir = normalize(-light.position);
	}
	else if (light.type == SpotLight)
	{
		lightDir = normalize(light.position - outFragPos);
		float theta = dot(lightDir, normalize(-light.direction));
		float epsilon = light.cutOff - light.outerCutOff;
		intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 0.5);  
	}
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, outTexCoords));

	//Specular light
	vec3 viewDir = normalize(uViewPosition - outFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, outTexCoords));

	//Attenuation
	if (light.type != Directional)
	{
		float dist = length(light.position - outFragPos);
		float attenuation = 1.0 / (light.constant * light.linear * dist * light.quadratic * (dist * dist));

		if (light.type == SpotLight)
		{
			diffuse *= intensity;
			specular *= intensity;
		}
		//else
		//	ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}
	//Final color
	vec3 result = (ambient + diffuse + specular);
	FragColor = vec4(result, 1.0);

	//FragColor = vec4(vec3(LinearizeDepth(gl_FragCoord.z) / far), 1.0);
}
