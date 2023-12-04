#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Light {
public:
	glm::vec3 AmbientColor;
	glm::vec3 DiffuseColor;
	glm::vec3 SpecularColor;

	glm::vec3 Position;

	Light(glm::vec3 position = glm::vec3(0.0f), glm::vec3 color = glm::vec3(1.0f), float ambientStrength = 0.2f, float diffuseStrength = 0.5f, float specularStrength = 1.0f){
		AmbientColor = color * ambientStrength;
		DiffuseColor = color * diffuseStrength;
		SpecularColor = color * specularStrength;
		Position = position;
	}
	Light(glm::vec3 ambientColor, glm::vec3 diffuseColor, glm::vec3 specularColor, glm::vec3 position = glm::vec3(0.0f)) {
		AmbientColor = ambientColor;
		DiffuseColor = diffuseColor;
		SpecularColor = specularColor;
		Position = position;
	}
	
private:

};

#endif