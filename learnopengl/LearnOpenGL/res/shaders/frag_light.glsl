#version 450 core

struct Material {
	sampler2D diffuse0;
	sampler2D diffuse1;
	sampler2D diffuse2;
	sampler2D specular0;
	sampler2D specular1;
	sampler2D emission;
	float shininess;
};

struct Light {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct DirectionalLight {
	Light light;
	vec3 direction;
};

struct PointLight {
	Light light;
	vec3 position;
	vec3 attenuation; // Attenuation constants
};

struct SpotLight {
	Light light;
	vec3 position;
	vec3 attenuation;
	vec3 direction;
	float cutoff; // angle in radians
};

#define NR_POINT_LIGHTS 2

layout(std140, binding=1) uniform FragLight {
	vec3 viewPos;
	bool reflective;
	bool refractive;
};

in GS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} fs_in;

uniform Material material;
uniform DirectionalLight dirLight;
uniform PointLight lights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform samplerCube skybox;

out vec4 FragColor;

vec4 getAmbient(Light light) {
	return vec4(light.ambient, 1.0) * texture(material.diffuse0, fs_in.texCoords);
}

vec3 getNegLightDir(vec3 lightPos, vec3 lightDir) {
	vec3 negLightDir = vec3(0.0);
	if (lightPos == vec3(0.0)) {
		negLightDir = normalize(-lightDir);
	} else {
		negLightDir = normalize(lightPos - fs_in.fragPos);
	}

	return negLightDir;
}

vec4 getDiffuse(Light light, vec3 lightPos, vec3 lightDir) {
	vec3 norm = normalize(fs_in.normal);
	vec3 negLightDir = getNegLightDir(lightPos, lightDir);

	float diffuseValue = max(dot(negLightDir, norm), 0.0);
	return vec4(light.diffuse, 1.0) * diffuseValue * texture(material.diffuse0, fs_in.texCoords);
}

vec4 getSpecular(Light light, vec3 lightPos, vec3 lightDir) {
	vec3 norm = normalize(fs_in.normal);
	if (lightDir == vec3(0.0)) {
		lightDir = -getNegLightDir(lightPos, lightDir);
	} else {
		lightDir = normalize(lightDir);
	}
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 reflectDir = reflect(lightDir, norm);

	float specularValue = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec4 specularMap = texture(material.specular0, fs_in.texCoords);
	return vec4(light.specular, 1.0) * specularValue * specularMap;
}

vec4 getDirectionalLight(DirectionalLight light) {
	vec3 negLightDir = normalize(light.direction);
	
	vec4 ambient = getAmbient(light.light);
	vec4 diffuse = getDiffuse(light.light, vec3(0.0), light.direction);
	vec4 specular = getSpecular(light.light, vec3(0.0), light.direction);
	return ambient + diffuse + specular;
}

vec4 getPointLight(PointLight light) {
	vec4 ambient = getAmbient(light.light);
	vec4 diffuse = getDiffuse(light.light, light.position, vec3(0.0));
	vec4 specular = getSpecular(light.light, light.position, vec3(0.0));

	vec3 a = light.attenuation;
	float dist = length(light.position - fs_in.fragPos);
	float attenuation = 1.0 / (a.x + a.y * dist + a.y * dist * dist);
	return ambient + (diffuse + specular) * attenuation;
}

vec4 getSpotLight(SpotLight light) {
	vec4 ambient = getAmbient(light.light);
	
	vec3 rayDir = normalize(vec3(light.position - fs_in.fragPos));
	float theta = dot(-light.direction, rayDir);

	if (theta < light.cutoff) {
		return ambient;
	}

	float intensity = clamp(1.0 - (1.0 - theta) / (1.0 - light.cutoff), 0.0, 1.0);
	vec4 diffuse = getDiffuse(light.light, vec3(0.0), light.direction) * intensity;
	vec4 specular = getSpecular(light.light, vec3(0.0), light.direction) * intensity;

	return (ambient + diffuse + specular);
}

const float ratio = 1.0 / 1.52;

void main()
{
	vec4 result = vec4(0.0);
	if (!reflective && !refractive) {
		result += getDirectionalLight(dirLight);
		for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
			result += getPointLight(lights[i]);
		}
		result += getSpotLight(spotLight);
		
		result /= (NR_POINT_LIGHTS + 2);
	} else {
		const vec3 I = normalize(fs_in.fragPos - viewPos);
		vec3 R = vec3(0.0);
		if (reflective) {
			R = reflect(I, normalize(fs_in.normal));
		}
		if (refractive) {
			R = refract(I, normalize(fs_in.normal), ratio);
		}
		result = vec4(texture(skybox, R).rgb, 1.0);
	}

	FragColor = result;
}