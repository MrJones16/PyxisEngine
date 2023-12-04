#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool useNormals;



void main()
{
    //gl position is the preset variable that is the output of the vertex shader
    vec4 position;
    if (useNormals){
        position = vec4(aPos + aNormal * 0.1f, 1f);
    }else{
        position = vec4(aPos, 1f);
    }
    gl_Position = projection * view * model * position;
    FragPos = vec3(model * vec4(aPos, 1.0f));
    TexCoords = aTexCoords;
    Normal = mat3(transpose(inverse(model))) * aNormal;
}