#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <renderer/Vertex.h>

using SkyboxHandle = uint32_t;
using MeshHandle = uint32_t;
using MaterialHandle = uint32_t;
using TextureHandle = uint32_t;
using PointLightHandle = uint32_t;

struct CameraData {
	glm::vec3 position;
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
};

struct RenderMesh {
	MeshHandle mesh;
	MaterialHandle material;
	glm::mat4 transform;
};


struct RenderSkybox {
    unsigned int cubemapTexture;
    glm::mat4 view;
    glm::mat4 projection;
};

struct RenderDirectionalLight
{
    glm::vec3 color = glm::vec3(1.f);

    float exposure = 0.5f;
    float ambientIntensity = 1.0f;
    float lightIntensity = 1.0f;

    glm::vec3 lightDir = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct RenderData {
    std::vector<RenderMesh> meshes;
    std::vector<PointLightHandle> lights;
    SkyboxHandle skybox = 0;
	CameraData camera;
    RenderDirectionalLight directionalLight;
};


// Struct for pointlight data representation
struct PointLightData
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

// Struct for material data representation
struct MaterialData 
{
    glm::vec3 albedo = glm::vec3(1.0f);
    float metallic = 0.0f;
    float roughness = 1.0f;
    float ao = 1.0f;
    glm::vec3 emissiveColor = glm::vec3(0.0f);

    TextureHandle albedoMap;
    TextureHandle normalMap;
    TextureHandle metallicMap;
    TextureHandle roughnessMap;
    TextureHandle aoMap;
    TextureHandle emissiveMap;
};

// Struct for mesh data representation
struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};