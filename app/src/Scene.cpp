#include "Scene.h"
#include <iostream>
#include <GLFW/glfw3.h>


Scene::Scene()
{

}

Scene::~Scene()
{
}

void Scene::update(float dt)
{
	/*for (auto entity : m_entities)
	{
		if (entity->pointLight) {
			float time = glfwGetTime();
			float amplitude = 120.0f;
			entity->position.x = amplitude * cos(time);
		}
	}*/
}


void Scene::addEntity(std::shared_ptr<Entity> entity) {
	m_entities.push_back(entity);
}

void Scene::deleteEntity(std::shared_ptr<Entity> entity) {
	m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
}

void Scene::addPointLight(PointLightHandle light)
{
	m_pointLights.push_back(light);
}

void Scene::deletePointLight(PointLightHandle light)
{
	m_pointLights.erase(std::remove(m_pointLights.begin(), m_pointLights.end(), light), m_pointLights.end());
}
