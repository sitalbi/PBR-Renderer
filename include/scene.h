#pragma once

#include "entity.h"

class Skybox;

class Scene {
public:
	Scene();
	~Scene();

	void addEntity(std::shared_ptr<Entity> entity);
	void deleteEntity(std::shared_ptr<Entity> entity);
	std::vector<std::shared_ptr<Entity>> getEntities() { return m_entities; }

	glm::vec3& getNewEntityPosition() { return m_newEntityPosition; }
	bool& getIsAddingEntity() { return m_isAddingEntity; }

	void draw(const glm::mat4& view, const glm::mat4& projection);

	void drawSkybox(const glm::mat4& view, const glm::mat4& projection);

private:
	std::vector<std::shared_ptr<Entity>> m_entities;

	glm::vec3 m_newEntityPosition = glm::vec3(0.0f);
	bool m_isAddingEntity = false;

	std::unique_ptr<Skybox> m_skybox;
	
};