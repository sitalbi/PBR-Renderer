#pragma once

#include "shader.h"
#include "texture.h"
#include <memory>

struct Material {
	static const int ALBEDO_TEXTURE_UNIT = 4;
	static const int NORMAL_TEXTURE_UNIT = 5;
	static const int METAL_TEXTURE_UNIT = 6;
	static const int ROUGH_TEXTURE_UNIT = 7;
	static const int AO_TEXTURE_UNIT = 8;
	static const int EMISSIVE_TEXTURE_UNIT = 9;

	// Material properties
	glm::vec3 albedo = glm::vec3(1.0f);
	float metallic = 0.0f;
	float roughness = 1.0f;
	float ao = 1.0f;
	glm::vec3 emissiveColor = glm::vec3(0.0f);

	// Textures (might use only ids instead of shared_ptr)
	std::shared_ptr<Texture> albedoMap;
	std::shared_ptr<Texture> normalMap;
	std::shared_ptr<Texture> metallicMap;
	std::shared_ptr<Texture> roughnessMap;
	std::shared_ptr<Texture> aoMap;
	std::shared_ptr<Texture> emissiveMap;

	// Textures flags
	bool useAlbedoMap = false;
	bool useNormalMap = false;
	bool useMetalMap = false;
	bool useRoughMap = false;
	bool useAoMap = false;
	bool useEmissiveMap = false;

	// Shader
	std::shared_ptr<Shader> shader;


	void bind()
	{
		if (!shader) return;
		shader->bind();

		// Set material properties to shader
		// Albedo
		shader->setUniform3f("material.albedo", albedo.x, albedo.y, albedo.z);
	
		// Metal Roughness
		shader->setUniform1f("material.metallic", metallic);
		shader->setUniform1f("material.roughness", roughness);
	
		// Ambient Occlusion
		shader->setUniform1f("material.ao", ao);
	
		// Emissive
		shader->setUniform3f("material.emissiveColor", emissiveColor.x, emissiveColor.y, emissiveColor.z);
	
	
		// Set material properties to shader
		// Albedo
		if (albedoMap) {
			albedoMap->bind(Material::ALBEDO_TEXTURE_UNIT);
			shader->setUniform1i("material.albedoMap", Material::ALBEDO_TEXTURE_UNIT);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			shader->setUniform1i("material.albedoMap", Material::ALBEDO_TEXTURE_UNIT);
		}
	
		// Normal
		if (normalMap) {
			normalMap->bind(Material::NORMAL_TEXTURE_UNIT);
			shader->setUniform1i("material.normalMap", Material::NORMAL_TEXTURE_UNIT);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			shader->setUniform1i("material.normalMap", Material::NORMAL_TEXTURE_UNIT);
		}
	
		// Metallic
		if (metallicMap) {
			metallicMap->bind(Material::METAL_TEXTURE_UNIT);
			shader->setUniform1i("material.metallicMap", Material::METAL_TEXTURE_UNIT);
		} 
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			shader->setUniform1i("material.metallicMap", Material::METAL_TEXTURE_UNIT);
		}
	
		// Roughness
		if (roughnessMap) {
			roughnessMap->bind(Material::ROUGH_TEXTURE_UNIT);
			shader->setUniform1i("material.roughnessMap", Material::ROUGH_TEXTURE_UNIT);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			shader->setUniform1i("material.roughnessMap", Material::ROUGH_TEXTURE_UNIT);
		}
	
	
		// Ambient Occlusion
		if (aoMap) {
			aoMap->bind(Material::AO_TEXTURE_UNIT);
			shader->setUniform1i("material.aoMap", Material::AO_TEXTURE_UNIT);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			shader->setUniform1i("material.aoMap", Material::AO_TEXTURE_UNIT);
		}

		// Emissive
		if (emissiveMap) {
			emissiveMap->bind(Material::EMISSIVE_TEXTURE_UNIT);
			shader->setUniform1i("material.emissiveMap", Material::EMISSIVE_TEXTURE_UNIT);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			shader->setUniform1i("material.emissiveMap", Material::EMISSIVE_TEXTURE_UNIT);
		}
	
		shader->setUniformBool("material.useAlbedoTexture", useAlbedoMap);
		shader->setUniformBool("material.useNormalTexture", useNormalMap);
		shader->setUniformBool("material.useMetallicTexture", useMetalMap);
		shader->setUniformBool("material.useRoughnessTexture", useRoughMap);
		shader->setUniformBool("material.useAoTexture", useAoMap);
		shader->setUniformBool("material.useEmissiveTexture", useEmissiveMap);
	}
};
