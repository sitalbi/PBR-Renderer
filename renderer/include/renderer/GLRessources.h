#pragma once
#include "glad/glad.h"
#include <glm/glm.hpp>



struct GLMesh {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    unsigned int indexCount;
};

struct GLTexture {
    unsigned int id;
    GLenum target = GL_TEXTURE_2D;
};

struct GLMaterial {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    glm::vec3 emissiveColor;

    unsigned int albedoMap;
    unsigned int normalMap;
    unsigned int metallicMap;
    unsigned int roughnessMap;
    unsigned int aoMap;
    unsigned int emissiveMap;
};

struct GLEnvironment {
    unsigned int hdrTexture = 0;
    unsigned int envCubemap = 0;
    unsigned int irradianceMap = 0;
    unsigned int prefilterMap = 0;
};

struct GLPointLight
{
    GLuint shadowCubemap = 0;
    uint32_t resolution = 1024;
    float nearPlane = 1.0f;
    float farPlane = 300.0f;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};