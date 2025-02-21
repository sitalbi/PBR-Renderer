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
