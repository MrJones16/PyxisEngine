#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include "shader_m.h"

class Light {
public:
	glm::vec3 AmbientColor;
	glm::vec3 DiffuseColor;
	glm::vec3 SpecularColor;
	Light(glm::vec3 color = glm::vec3(1.0f), float ambientStrength = 0.05f, float diffuseStrength = 0.5f, float specularStrength = 1.0f) {
		AmbientColor = color * ambientStrength;
		DiffuseColor = color * diffuseStrength;
		SpecularColor = color * specularStrength;
	}
	virtual void UpdateShader(Shader shader, std::string varName) {
		shader.setVec3(varName + ".ambient", AmbientColor);
		shader.setVec3(varName + ".diffuse", DiffuseColor);
		shader.setVec3(varName + ".ambient", SpecularColor);
	}
	virtual std::string getType() {
		return "Light";
	}
};

class DirectionalLight  : public Light {
public:
	glm::vec3 Direction;

	DirectionalLight(glm::vec3 direction = glm::vec3(0,0,-1.0f), glm::vec3 color = glm::vec3(1.0f), float ambientStrength = 0.05f, float diffuseStrength = 0.5f, float specularStrength = 1.0f){
		AmbientColor = color * ambientStrength;
		DiffuseColor = color * diffuseStrength;
		SpecularColor = color * specularStrength;
		Direction = direction;
	}
	void UpdateShader(Shader shader, std::string varName) override {
		shader.setVec3(varName + ".ambient", AmbientColor);
		shader.setVec3(varName + ".diffuse", DiffuseColor);
		shader.setVec3(varName + ".specular", SpecularColor);
		shader.setVec3(varName + ".direction", Direction);
	}
	std::string getType() override {
		return "DirectionalLight";
	}
};

class PointLight : public Light {
public:
	glm::vec3 Position;

	float Constant;
	float Linear;
	float Quadratic;

	PointLight(glm::vec3 position = glm::vec3(0, 0, -1.0f), glm::vec3 color = glm::vec3(1.0f),
		float ambientStrength = 0.05f, float diffuseStrength = 0.5f, float specularStrength = 1.0f,
		float constant = 1, float linear = 0.09f, float quadratic = 0.032f) {
		Position = position;
		
		Constant = constant;
		Linear = linear;
		Quadratic = quadratic;

		AmbientColor = color * ambientStrength;
		DiffuseColor = color * diffuseStrength;
		SpecularColor = color * specularStrength;
	}
	void UpdateShader(Shader shader, std::string varName) override {
		shader.setVec3(varName + ".position", Position);

		shader.setFloat(varName + ".constant",  Constant);
		shader.setFloat(varName + ".linear",    Linear);
		shader.setFloat(varName + ".quadratic", Quadratic);

		shader.setVec3(varName + ".ambient", AmbientColor);
		shader.setVec3(varName + ".diffuse", DiffuseColor);
		shader.setVec3(varName + ".specular", SpecularColor);
		
	}
	std::string getType() override {
		return "PointLight";
	}
};

class SpotLight : public Light {
public:
	glm::vec3 Position;
	glm::vec3 Direction;

	float Constant;
	float Linear;
	float Quadratic;

	float cutOff;
	float outerCutOff;

	SpotLight(glm::vec3 position = glm::vec3(0, 0, -1.0f), glm::vec3 direction = glm::vec3(0,0,-1.0f), glm::vec3 color = glm::vec3(1.0f),
		float ambientStrength = 0.05f, float diffuseStrength = 0.5f, float specularStrength = 1.0f,
		float constant = 1, float linear = 0.09f, float quadratic = 0.032f, float cutoff = 0.9763f, float outercutoff = 0.9659f) {
		Position = position;
		Direction = direction;

		Constant = constant;
		Linear = linear;
		Quadratic = quadratic;

		cutOff = cutoff;
		outerCutOff = outercutoff;

		AmbientColor = color * ambientStrength;
		DiffuseColor = color * diffuseStrength;
		SpecularColor = color * specularStrength;
	}
	void UpdateShader(Shader shader, std::string varName) override {
		shader.setVec3(varName + ".position",  Position);
		shader.setVec3(varName + ".direction", Direction);

		shader.setFloat(varName + ".constant",  Constant);
		shader.setFloat(varName + ".linear",    Linear);
		shader.setFloat(varName + ".quadratic", Quadratic);

		shader.setFloat(varName + ".cutOff",      cutOff);
		shader.setFloat(varName + ".outerCutOff", outerCutOff);

		shader.setVec3(varName + ".ambient", AmbientColor);
		shader.setVec3(varName + ".diffuse", DiffuseColor);
		shader.setVec3(varName + ".specular", SpecularColor);

	}
	std::string getType() override {
		return "SpotLight";
	}
};


#endif