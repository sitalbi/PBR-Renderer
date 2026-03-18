#pragma once

#include "Entity.h"

class Skybox;


class Scene {
public:
	Scene();
	~Scene();

	void addEntity(std::shared_ptr<Entity> entity);
	void deleteEntity(std::shared_ptr<Entity> entity);

	void addPointLight(PointLightHandle light);
	void deletePointLight(PointLightHandle light);

	std::vector<std::shared_ptr<Entity>> getEntities() { return m_entities; }
	std::vector<PointLightHandle>& getPointLights(){ return m_pointLights; }

	glm::vec3& getNewEntityPosition() { return m_newEntityPosition; }
	bool& getIsAddingEntity() { return m_isAddingEntity; }


	void update(float dt);

	SkyboxHandle skyboxHandle;
	RenderDirectionalLight dirLight;
	

private:
	std::vector<std::shared_ptr<Entity>> m_entities;
	std::vector<PointLightHandle> m_pointLights;

	glm::vec3 m_newEntityPosition = glm::vec3(0.0f);
	bool m_isAddingEntity = false;


	int m_pointLightsNum = 0;
	
};