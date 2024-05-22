#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int isOverLay;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0f)); //We multiply by model so that the objects's lighting is calculated in the context of the world
    Normal = mat3(transpose(inverse(model))) * aNormal; //When the object transforms using model matrix, aNormal doesent change, so we use Normal to keep track of how the normal should actually be like when the object transforms
    gl_Position = projection * view *  vec4(FragPos, 1.0f);
    TexCoord = aTexCoord;
}