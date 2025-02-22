#include "entity.h"
#include <glm/ext/matrix_transform.hpp>

Entity::Entity()
{
}

void Entity::draw(const glm::mat4& view, const glm::mat4& projection)
{
	if (!m_mesh) {
		return;
	}

	if (useMaterial) {
		m_material.shader->bind();
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		modelMatrix = glm::scale(modelMatrix, scale);

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

		m_material.shader->setUniformMat4f("model", modelMatrix);
		m_material.shader->setUniformMat4f("view", view);
		m_material.shader->setUniformMat4f("projection", projection);
		m_material.shader->setUniformMat3f("normalMatrix", normalMatrix);

		// Set material properties to shader
		
		// Albedo
		m_material.shader->setUniform3f("albedo", m_material.albedo.x, m_material.albedo.y, m_material.albedo.z);

		// Metal Roughness
		m_material.shader->setUniform1f("metallic", m_material.metallic);
		m_material.shader->setUniform1f("roughness", m_material.roughness);

		// Ambient Occlusion
		m_material.shader->setUniform1f("ao", m_material.ao);

		// Emissive (Not implemented yet)
		//m_material.shader->setUniform3f("emissiveColor", m_material.emissiveColor.x, m_material.emissiveColor.y, m_material.emissiveColor.z);


		//// Set material properties to shader (not implemented yet)
		//
		//// Albedo
		//m_material.shader->setUniformBool("useAlbedoMap", m_material.useAlbedoMap);
		//if (m_material.useAlbedoMap && m_material.albedoMap) {
		//	m_material.albedoMap->bind(ALBEDO_TEXTURE_UNIT);
		//	m_material.shader->setUniform1i("albedoMap", ALBEDO_TEXTURE_UNIT);
		//}
		//else {
		//	m_material.shader->setUniform3f("albedo", m_material.albedo);
		//}

		//// Normal
		//m_material.shader->setUniformBool("useNormalMap", m_material.useNormalMap);
		//if (m_material.useNormalMap && m_material.normalMap) {
		//	m_material.normalMap->bind(NORMAL_TEXTURE_UNIT);
		//	m_material.shader->setUniform1i("normalMap", NORMAL_TEXTURE_UNIT);
		//}

		//// Metal Roughness
		//m_material.shader->setUniformBool("useMetalRoughMap", m_material.useMetalRoughMap);
		//if (m_material.useMetalRoughMap && m_material.metalRoughMap) {
		//	m_material.metalRoughMap->bind(METAL_ROUGH_TEXTURE_UNIT);
		//	m_material.shader->setUniform1i("metalRoughMap", METAL_ROUGH_TEXTURE_UNIT);
		//}
		//m_material.shader->setUniform1f("metallic", m_material.metallic);
		//m_material.shader->setUniform1f("roughness", m_material.roughness);

		//// Ambient Occlusion
		//m_material.shader->setUniformBool("useAoMap", m_material.useAoMap);
		//if (m_material.useAoMap && m_material.aoMap) {
		//	m_material.aoMap->bind(AO_TEXTURE_UNIT);
		//	m_material.shader->setUniform1i("aoMap", AO_TEXTURE_UNIT);
		//}
		//else {
		//	m_material.shader->setUniform1f("ao", m_material.ao);
		//}

		//// Emissive
		//m_material.shader->setUniformBool("useEmissiveMap", m_material.useEmissiveMap);
		//if (m_material.useEmissiveMap && m_material.emissiveMap) {
		//	m_material.emissiveMap->bind(EMISSIVE_TEXTURE_UNIT);
		//	m_material.shader->setUniform1i("emissiveMap", EMISSIVE_TEXTURE_UNIT);
		//}
		//else {
		//	m_material.shader->setUniform3f("emissiveColor", m_material.emissiveColor);
		//}
	}

	m_mesh->draw();
}
