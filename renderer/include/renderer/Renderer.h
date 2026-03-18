#pragma once

#include <vector>
#include <memory>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <renderer/Framebuffer.h>
#include <renderer/GLRessources.h>
#include <renderer/RenderData.h>
#include <renderer/Shader.h>

#define window_width 1920
#define window_height 1080

class BloomRenderer;

struct PointShadowDirection
{
    GLenum CubemapFace;
    glm::vec3 Target;
    glm::vec3 Up;
};

constexpr PointShadowDirection pointShadowDirection[6] =
{
    { GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec3(0.0f, -1.0f,  0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f,  0.0f, 1.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec3(0.0f,  0.0f, -1.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f,  0.0f,  1.0f),  glm::vec3(0.0f, -1.0f,  0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, -1.0f,  0.0f) }
};

struct RenderSize {
    int width;
    int height;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void init();
    void updateLighting(const RenderDirectionalLight& light);
	void shutdown();

	void setLightColor(const glm::vec3& lightColor) { m_lightColor = lightColor; }


	std::shared_ptr<Shader> getBasicShader() { return m_basicShader; }
	std::shared_ptr<Shader> getPBRShader() { return m_pbrShader; }

	void render(const RenderData& renderData);

    static void renderQuad() {
        if (quadVAO == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    static void renderCube()
    {
        // initialize (if necessary)
        if (cubeVAO == 0)
        {
            float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
                 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
                 // bottom face
                 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                 // top face
                 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
                  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
            };
            glGenVertexArrays(1, &cubeVAO);
            glGenBuffers(1, &cubeVBO);
            // fill buffer
            glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            // link vertex attributes
            glBindVertexArray(cubeVAO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        // render Cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    SkyboxHandle createSkyboxFromHDR(const std::string& path);
    TextureHandle createTextureFromFile(const std::string& path);
    GLTexture loadTextureFromFile(const char* path);
    PointLightHandle createPointLight(PointLightData data);

    void setupMesh(GLMesh& mesh, MeshData& meshDesc);

    MeshHandle createMesh(MeshData meshDesc);
    MaterialHandle createMaterial(MaterialData material);

    void editMaterial(MaterialHandle handle, const MaterialData& data);
    MaterialData getMaterialData(MaterialHandle handle);

    void editPointLight(PointLightHandle handle, const PointLightData& data);
    PointLightData getPointLightData(PointLightHandle handle);


	bool useSSAO = true;
	bool useBloom = true;
	bool useAA = true;

    float bloomFilterRadius = 0.0025f;



    RenderSize m_renderTargetResolution{ 1920,1080 };

    static constexpr int ALBEDO_TEXTURE_UNIT = 4;
    static constexpr int NORMAL_TEXTURE_UNIT = 5;
    static constexpr int METAL_TEXTURE_UNIT = 6;
    static constexpr int ROUGH_TEXTURE_UNIT = 7;
    static constexpr int AO_TEXTURE_UNIT = 8;
    static constexpr int EMISSIVE_TEXTURE_UNIT = 9;

private:
    static unsigned int cubeVAO;
    static unsigned int cubeVBO;
    static unsigned int quadVAO;
    static unsigned int quadVBO;

    unsigned int m_skyboxVAO; 
    unsigned int m_skyboxVBO;

    unsigned int m_captureFBO;
    unsigned int m_captureRBO;

    unsigned int m_brdfLUTTexture;
    unsigned int m_defaultWhiteTexture;
    unsigned int m_defaultFlatNormalTexture;

	glm::vec3 m_lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	bool m_initialized = false;

	std::shared_ptr<Shader> m_basicShader;
	std::shared_ptr<Shader> m_depthShader;
	std::shared_ptr<Shader> m_pointDepthShader;
	std::shared_ptr<Shader> m_pbrShader;
	std::unique_ptr<Shader> m_lightingShader;
	std::unique_ptr<Shader> m_ssaoShader;
	std::unique_ptr<Shader> m_ssaoBlurShader;
	std::unique_ptr<Shader> m_brightShader;
	std::unique_ptr<Shader> m_finalCompoShader;
	std::unique_ptr<Shader> m_skyboxShader;

    // Skybox ibl shaders
    std::unique_ptr<Shader> m_equirectangularToCubemapShader;
    std::unique_ptr<Shader> m_irradianceShader;
    std::unique_ptr<Shader> m_prefilterShader;
    std::unique_ptr<Shader> m_brdfShader;

    std::unique_ptr<Framebuffer> m_backgroundFB;
	std::unique_ptr<Framebuffer> m_depthFB; 
	std::unique_ptr<Framebuffer> m_pointShadowFB;
    std::unique_ptr<Framebuffer> m_geometryFB_MSAA;
    std::unique_ptr<Framebuffer> m_geometryFB;
	std::unique_ptr<Framebuffer> m_ssaoFB;
	std::unique_ptr<Framebuffer> m_ssaoBlurFB;
	std::unique_ptr<Framebuffer> m_brightFB;
	std::unique_ptr<Framebuffer> m_hdrFB;
	std::unique_ptr<Framebuffer> m_finalCompositeFB;

	std::unique_ptr<BloomRenderer> m_bloomRenderer;

	unsigned int m_ssaoNoiseTexture;
    std::vector<glm::vec3> ssaoKernel;

    // GL Ressources handles lookups
    std::unordered_map<MeshHandle, GLMesh> m_meshes;
    std::unordered_map<MaterialHandle, MaterialData> m_materialData;
    std::unordered_map<MaterialHandle, GLMaterial> m_materials;
    std::unordered_map<TextureHandle, GLTexture> m_textures;
    std::unordered_map<SkyboxHandle, GLEnvironment> m_skyboxes;

    std::unordered_map<PointLightHandle, GLPointLight> m_pointLights;

    // Default meshes
    MeshHandle cubeMeshHandle;
    MeshHandle sphereMeshHandle;

	void clear();

    void bindMaterial(const Shader& shader, const GLMaterial& material);

    unsigned int createSolidTextureRGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    void initSkybox();
    unsigned int loadHDRImage(std::string path);
    GLEnvironment buildEnvironmentMaps(unsigned int hdrTexture);

};

struct BloomMip
{
    glm::vec2 size;
    glm::ivec2 intSize;
    unsigned int texture;
};

class BloomRenderer
{
public:
    BloomRenderer();
    ~BloomRenderer();
    bool init(unsigned int windowWidth, unsigned int windowHeight, unsigned int numMips);
    void destroy();
    void renderBloomTexture(unsigned int srcTexture, float filterRadius);
    unsigned int bloomTexture();

private:
    void renderDownsamples(unsigned int srcTexture);
    void renderUpsamples(float filterRadius);

    bool m_init = false;
    glm::ivec2 m_srcViewportSize;
    glm::vec2 m_srcViewportSizeFloat;
    std::unique_ptr<Shader> m_downsampleShader;
    std::unique_ptr<Shader> m_upsampleShader;

	std::vector<BloomMip> m_mipChain;

	unsigned int m_bloomFBO;
	unsigned int m_depthBuffer;
};
