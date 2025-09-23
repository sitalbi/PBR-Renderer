#include "scene.h"
#include <iostream>
#include <skybox.h>

Scene::Scene()
{
	m_skybox = std::make_unique<Skybox>();
	// Load default skybox cubemap hdr texture
	m_skybox->loadHDRImage(RES_DIR"/textures/skybox/brown_photostudio_02_4k.hdr");
}

Scene::~Scene()
{
}

void Scene::draw(std::shared_ptr<Shader> shader, const glm::mat4& view, const glm::mat4& projection)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);


	if (m_skybox) {
		m_skybox->bindTextures();

		// Set IBL uniforms
		shader->bind();
		shader->setUniform1i("irradianceMap", 0);
		shader->setUniform1i("prefilterMap", 1);
		shader->setUniform1i("brdfLUT", 2);
	}

	// Set point lights uniforms
	shader->setUniform1i("numPointLights", m_pointLightsNum);

	int i = 0;
	// Draw entities
	for (auto& entity : m_entities)
	{
		// If entity has a point light component, update its position in the point lights array
		if (entity->pointLight) {
			std::string idx = "pointLights[" + std::to_string(i) + "]";
			shader->setUniformVec3f(idx + ".position", entity->position);
			shader->setUniformVec3f(idx + ".color", entity->pointLight->color);
			shader->setUniform1f(idx + ".intensity", entity->pointLight->intensity);
			shader->setUniform1f(idx + ".constant", entity->pointLight->constant);
			shader->setUniform1f(idx + ".linear", entity->pointLight->linear);
			shader->setUniform1f(idx + ".quadratic", entity->pointLight->quadratic);
			++i;
		}

		entity->draw(shader, view, projection);
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
	if (entity->pointLight) {
		m_pointLightsNum++;
	}
}

void Scene::deleteEntity(std::shared_ptr<Entity> entity) {
	m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
	if (entity->pointLight) {
		m_pointLightsNum--;
	}
}