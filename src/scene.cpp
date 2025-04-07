#include "scene.h"
#include <iostream>
#include <skybox.h>

Scene::Scene()
{
	m_skybox = std::make_unique<Skybox>();
	// Load default skybox cubemap hdr texture
	m_skybox->loadHDRImage(RES_DIR"/textures/skybox/kiara_1_dawn_4k.hdr");
}

Scene::~Scene()
{
}

void Scene::draw(const glm::mat4& view, const glm::mat4& projection)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	if (m_skybox) {
		m_skybox->bindTextures();
	}

	// Draw entities
	for (auto& entity : m_entities)
	{
		entity->draw(view, projection);
	}
}

void Scene::drawSkybox(const glm::mat4& view, const glm::mat4& projection)
{
	if (m_skybox) {
		m_skybox->draw(view, projection);
	}
}


void Scene::addEntity(std::shared_ptr<Entity> entity) {
	m_entities.push_back(entity);
}

void Scene::deleteEntity(std::shared_ptr<Entity> entity) {
	m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
}