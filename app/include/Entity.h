#pragma once

#include <memory>
#include <Model.h>
#include <renderer/RenderData.h>

class Entity {
public:
	Entity();
	Entity(std::shared_ptr<Model> model, glm::vec3 position, std::string name = "new entity") : m_model(model), position(position), m_name(name) 
	{}

	void setMaterial(int submeshIndex, MaterialHandle material)
	{
		materialOverrides[submeshIndex] = material;
	}

	bool hasMaterialOverride(int submeshIndex) const
	{
		return materialOverrides.find(submeshIndex) != materialOverrides.end();
	}

	bool isUsingMaterial() { return materialOverrides.size() > 0; }

	MaterialHandle getMaterialOverride(int submeshIndex) const
	{
		return materialOverrides.at(submeshIndex);
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
	std::unordered_map<int, MaterialHandle> materialOverrides;

	std::string m_name;
};