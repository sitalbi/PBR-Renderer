#pragma once

#include "mesh.h"
#include "material.h"

#include <memory>
#include <model.h>

struct PointLight {
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;

	// Attenuation parameters
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	bool hasMesh = true;


	unsigned int shadowCubemap = 0;
	const float shadowFarPlane = 300.0f;

	PointLight() {
		glGenTextures(1, &shadowCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F,
				1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
	}

	~PointLight() {
		if (shadowCubemap) {
			glDeleteTextures(1, &shadowCubemap);
		}
	}
};

class Entity {
public:
	Entity();
	Entity(std::shared_ptr<Model> model, glm::vec3 position, std::string name = "new entity") : m_model(model), position(position), m_name(name) 
	{}

	void draw(std::shared_ptr<Shader> shader, const glm::mat4& view, const glm::mat4& projection);

	void drawModel() {
		if (m_model) {

			for (auto& mesh : m_model->getSubmeshes()) {
				mesh.draw();
			}
		}
	}

	void setMaterial(int submeshIndex, const Material& material) {
		if (submeshIndex >= 0 && submeshIndex < m_model->getSubmeshes().size()) {
			materialOverrides[submeshIndex] = material;
		}
	}

	bool isUsingMaterial() { return materialOverrides.size() > 0; }

	Material& getMaterial(int submeshIndex) {
		return materialOverrides[submeshIndex];
	}

	glm::mat4 getModelMatrix();

	std::shared_ptr<Model> getModel() { return m_model; }

	void setName(std::string name) { m_name = name; }
	std::string getName() { return m_name; }

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	std::shared_ptr<PointLight> pointLight = nullptr;

private:
	std::shared_ptr<Model> m_model;
	std::unordered_map<int, Material> materialOverrides;

	std::shared_ptr<Shader> m_basicShader;

	std::string m_name;
};