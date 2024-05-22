#version 440 core

struct Material {
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

uniform sampler2D diffuse0;
uniform sampler2D specular0;

uniform int noTex;

#define MAX_POINT_LIGHT 20;
struct PointLight {
    //attenuation
    float k0;
    float k1;
    float k2;

    vec3 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
uniform PointLight pointLights[20];
uniform int noPointLights;

#define MAX_SPOT_LIGHT = 5;
struct SpotLight {
    //attenuation
    float k0;
    float k1;
    float k2;

    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    vec4 diffuse; 
    vec4 ambient;
    vec4 specular;
};
uniform SpotLight spotLights[5];
uniform int noSpotLights;


struct DirLight {
    vec3 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform  DirLight dirLight;

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform Material material;

uniform vec3 viewPos;

vec4 calcDirLight(vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap);
vec4 calcPointLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap);
vec4 calcSpotLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap);

void main() {

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec4 diffMap;
    vec4 specMap;

    if(noTex == 1) { 
        // use materials
        diffMap = material.diffuse;
        specMap = material.specular;
    } else {
        diffMap = texture(diffuse0, TexCoord);
        specMap = texture(specular0, TexCoord);
    }

    //placeholder
    vec4 result;

    //directional light
    result = calcDirLight(norm, viewDir, diffMap, specMap);

    // point lights
    for(int i = 0; i < noPointLights; i++) {
        result += calcPointLight(i, norm, viewDir, diffMap, specMap);
    }

    //spot lights
    for(int i = 0; i < noSpotLights; i++) {
        result += calcSpotLight(i, norm, viewDir, diffMap, specMap);
    }

    FragColor = result; 
}

vec4 calcDirLight(vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap) {
    
    //ambient
    vec4 ambient = dirLight.ambient * diffMap;

    //diffuse
    vec3 lightDir = normalize(-dirLight.direction);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec4 diffuse = dirLight.diffuse * (diff * diffMap);

    //specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess * 128);
    vec4 specular = dirLight.specular * (spec * specMap);

    return vec4(ambient + diffuse + specular);
}

vec4 calcPointLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap) {

    //ambient
    vec4 ambient = pointLights[idx].ambient * diffMap;

    //diffuse
    vec3 lightDir = normalize(pointLights[idx].position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = pointLights[idx].diffuse * (diff * diffMap);

    //specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
    vec4 specular = pointLights[idx].specular * (spec * specMap);

    float dist = length(pointLights[idx].position - FragPos);
    float attenuation = 1.0 / (pointLights[idx].k0 + pointLights[idx].k1 * dist + pointLights[idx].k2 * (dist * dist)); 

   return vec4(ambient + diffuse + specular) * attenuation;
}

vec4 calcSpotLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap) {

    vec3 lightDir = normalize(spotLights[idx].position - FragPos);
    float theta = dot(lightDir, normalize(-spotLights[idx].direction));

    vec4 ambient = spotLights[idx].ambient * diffMap;

    if(theta > spotLights[idx].outerCutOff) { // > cause using cosines not degeres
        //if in cutOff
        //diffuse
        float diff = max(dot(norm, lightDir), 0.0f);
        vec4 diffuse = spotLights[idx].diffuse * (diff * diffMap);

        //specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
        vec4 specular = spotLights[idx].specular * (spec * specMap);

        float intensity = (theta - spotLights[idx].outerCutOff) / (spotLights[idx].cutOff - spotLights[idx].outerCutOff);
        intensity = clamp(intensity, 0.0, 1.0);
        diffuse *= intensity;
        specular *= intensity;

        float dist = length(spotLights[idx].position - FragPos);
        float attenuation = 1.0 / (spotLights[idx].k0 + spotLights[idx].k1 * dist + spotLights[idx].k2 * (dist * dist)); 

        return vec4(ambient + diffuse + specular) * attenuation;

    } else {
        //render just ambient
        return ambient;
    }

}