#pragma once

#include "mesh.h"
#include "material.h"

#include <memory>
#include <model.h>

class Entity {
public:
	Entity();
	Entity(std::shared_ptr<Model> model, glm::vec3 position, std::string name = "new entity") : m_model(model), position(position), m_name(name) 
	{}

	void draw(std::shared_ptr<Shader> shader, const glm::mat4& view, const glm::mat4& projection);

	void drawModel() {
		if (m_model) {

			m_model->draw();
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

private:
	std::shared_ptr<Model> m_model;
	std::unordered_map<int, Material> materialOverrides;

	std::shared_ptr<Shader> m_basicShader;

	std::string m_name;
};