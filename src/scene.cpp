#include "scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::draw(const glm::mat4& view, const glm::mat4& projection)
{
	for (auto& entity : m_entities)
	{
		entity->draw(view,projection);
	}
}

void Scene::addEntity(std::shared_ptr<Entity> entity) {
	m_entities.push_back(entity);
	entity->setName("Entity" + std::to_string(m_entities.size()));
}

void Scene::deleteEntity(std::shared_ptr<Entity> entity) {
	m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
}