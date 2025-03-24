#include "entity.h"
#include <glm/ext/matrix_transform.hpp>

Entity::Entity()
{
}

void Entity::draw(const glm::mat4& view, const glm::mat4& projection, unsigned int irradianceMap, unsigned int prefilterMap, unsigned int brdfLUT)
{
	if (!m_mesh) {
		return;
	}
	m_material.shader->bind();

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	modelMatrix = glm::scale(modelMatrix, scale);

	m_material.shader->setUniformMat4f("model", modelMatrix);
	m_material.shader->setUniformMat4f("view", view);
	m_material.shader->setUniformMat4f("projection", projection);

	// IBL
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	m_material.shader->setUniform1i("irradianceMap", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	m_material.shader->setUniform1i("prefilterMap", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	m_material.shader->setUniform1i("brdfLUT", 2);

	

	// Set material properties to shader
	// Albedo
	m_material.shader->setUniform3f("material.albedo", m_material.albedo.x, m_material.albedo.y, m_material.albedo.z);

	// Metal Roughness
	m_material.shader->setUniform1f("material.metallic", m_material.metallic);
	m_material.shader->setUniform1f("material.roughness", m_material.roughness);

	// Ambient Occlusion
	m_material.shader->setUniform1f("material.ao", m_material.ao);

	//// Emissive (Not implemented yet)
	////m_material.shader->setUniform3f("emissiveColor", m_material.emissiveColor.x, m_material.emissiveColor.y, m_material.emissiveColor.z);


	// Set material properties to shader
	// Albedo
	if (m_material.albedoMap) {
		m_material.albedoMap->bind(Material::ALBEDO_TEXTURE_UNIT);
		m_material.shader->setUniform1i("material.albedoMap", Material::ALBEDO_TEXTURE_UNIT);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_material.shader->setUniform1i("material.albedoMap", Material::ALBEDO_TEXTURE_UNIT);
	}

	// Normal
	if (m_material.normalMap) {
		m_material.normalMap->bind(Material::NORMAL_TEXTURE_UNIT);
		m_material.shader->setUniform1i("material.normalMap", Material::NORMAL_TEXTURE_UNIT);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_material.shader->setUniform1i("material.normalMap", Material::NORMAL_TEXTURE_UNIT);
	}

	// Metallic
	if (m_material.metallicMap) {
		m_material.metallicMap->bind(Material::METAL_TEXTURE_UNIT);
		m_material.shader->setUniform1i("material.metallicMap", Material::METAL_TEXTURE_UNIT);
	} 
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_material.shader->setUniform1i("material.metallicMap", Material::METAL_TEXTURE_UNIT);
	}

	// Roughness
	if (m_material.roughnessMap) {
		m_material.roughnessMap->bind(Material::ROUGH_TEXTURE_UNIT);
		m_material.shader->setUniform1i("material.roughnessMap", Material::ROUGH_TEXTURE_UNIT);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_material.shader->setUniform1i("material.roughnessMap", Material::ROUGH_TEXTURE_UNIT);
	}


	// Ambient Occlusion
	if (m_material.aoMap) {
		m_material.aoMap->bind(Material::AO_TEXTURE_UNIT);
		m_material.shader->setUniform1i("material.aoMap", Material::AO_TEXTURE_UNIT);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_material.shader->setUniform1i("material.aoMap", Material::AO_TEXTURE_UNIT);
	}

	// Emissive (Not implemented yet)
	/*m_material.shader->setUniformBool("useEmissiveMap", m_material.useEmissiveMap);
	if (m_material.useEmissiveMap && m_material.emissiveMap) {
		m_material.emissiveMap->bind(Material::EMISSIVE_TEXTURE_UNIT);
		m_material.shader->setUniform1i("emissiveMap", Material::EMISSIVE_TEXTURE_UNIT);
	}*/

	m_material.shader->setUniformBool("material.useAlbedoTexture", m_material.useAlbedoMap);
	m_material.shader->setUniformBool("material.useNormalTexture", m_material.useNormalMap);
	m_material.shader->setUniformBool("material.useMetallicTexture", m_material.useMetalMap);
	m_material.shader->setUniformBool("material.useRoughnessTexture", m_material.useRoughMap);
	m_material.shader->setUniformBool("material.useAoTexture", m_material.useAoMap);

	m_mesh->draw();
}
