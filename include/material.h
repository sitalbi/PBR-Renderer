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
	float ao = 0.25f;
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
};
