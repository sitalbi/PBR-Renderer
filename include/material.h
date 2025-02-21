#pragma once

#include "shader.h"
//#include "texture.h"
#include <memory>

const int ALBEDO_TEXTURE_UNIT = 0;
const int NORMAL_TEXTURE_UNIT = 1;
const int METAL_ROUGH_TEXTURE_UNIT = 2;
const int AO_TEXTURE_UNIT = 3;
const int EMISSIVE_TEXTURE_UNIT = 4;


struct Material {
	// Material properties
	glm::vec3 albedo = glm::vec3(1.0f);
	float metallic = 0.0f;
	float roughness = 1.0f;
	float ao = 1.0f;
	glm::vec3 emissiveColor = glm::vec3(0.0f);

	// Textures (not implemented yet)
	/*std::shared_ptr<Texture> albedoMap;
	std::shared_ptr<Texture> normalMap;
	std::shared_ptr<Texture> metalRoughMap;
	std::shared_ptr<Texture> aoMap;
	std::shared_ptr<Texture> emissiveMap;*/

	// Textures flags
	bool useAlbedoMap = false;
	bool useNormalMap = false;
	bool useMetalRoughMap = false;
	bool useAoMap = false;
	bool useEmissiveMap = false;

	// Shader
	std::shared_ptr<Shader> shader;
};
