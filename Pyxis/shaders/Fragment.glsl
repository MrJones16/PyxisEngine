#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 ViewPos;//

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float     shininess;
};

struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);  

struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


struct SpotLight {
    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

uniform Material material;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform DirLight dirLight; 
#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

float near = 0.1f;
float far = 100;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}
//float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
//FragColor = vec4(vec3(depth), 1.0);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(ViewPos - FragPos);

    //Directional Lighting
    vec4 result = CalcDirLight(dirLight, norm, viewDir);

    //Point Lighting
    for (int i = 0; i < NR_POINT_LIGHTS; i++){
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    // Spotlight Lighting
    //vec4 result = CalcSpotLight(spotLight, norm, FragPos, viewDir);
    //if (result.a < 0.5f) discard;

    FragColor = result;
    
}

vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * texture(material.texture_diffuse1, TexCoords).xyz;
    vec3 diffuse  = light.diffuse  * diff * texture(material.texture_diffuse1, TexCoords).xyz;
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoords).xyz;
    return vec4(ambient + diffuse + specular, texture(material.texture_diffuse1, TexCoords).a);
}  

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			                    light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * texture(material.texture_diffuse1, TexCoords).xyz;
    vec3 diffuse  = light.diffuse  * diff * texture(material.texture_diffuse1, TexCoords).xyz;
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoords).xyz;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return vec4(ambient + diffuse + specular, texture(material.texture_diffuse1, TexCoords).a);
} 

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    //diffuse
    float diff = max(dot(normal, lightDir), 0.0f);

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec      = pow(max(dot(viewDir, reflectDir),0.0f), material.shininess);

    //attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			            light.quadratic * (distance * distance));    

    //cutoff
    float theta     = dot(lightDir, normalize(-light.direction)); // cos of angle between light direction and direction to light
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 

    // combine results
    vec3 ambient  = light.ambient  * texture(material.texture_diffuse1, TexCoords).xyz;
    vec3 diffuse  = light.diffuse  * diff * texture(material.texture_diffuse1, TexCoords).xyz;
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoords).xyz;

    ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;

    return vec4(ambient + diffuse + specular, texture(material.texture_diffuse1, TexCoords).a);
}