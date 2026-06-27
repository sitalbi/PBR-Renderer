#include <algorithm>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <renderer/Renderer.h>
#include <renderer/RendererPrimitives.h>
#include <renderer/Texture.h>
#include <renderer/Vertex.h>
#include <renderer/Shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int Renderer::quadVAO = 0;
unsigned int Renderer::quadVBO = 0;
unsigned int Renderer::cubeVAO = 0;
unsigned int Renderer::cubeVBO = 0;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
	if (m_initialized)
	{
		shutdown();
	}
}

void Renderer::init()
{
	if (m_initialized)
	{
		return;
	}

	// OpenGL settings
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	glDisable(GL_CULL_FACE);

	// Skybox settings
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	m_pbrShader = std::make_shared<Shader>(RES_DIR "/shaders/basic_vert.glsl", RES_DIR  "/shaders/pbr_frag.glsl");
	if (m_pbrShader)
	{
		m_pbrShader->bind();
		glm::vec3 camPos = glm::vec3(0.0f);
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z);
	}

	m_depthShader = std::make_shared<Shader>(RES_DIR "/shaders/depth_vert.glsl", RES_DIR "/shaders/empty_frag.glsl");
	m_pointDepthShader = std::make_shared<Shader>(RES_DIR "/shaders/point_depth_vert.glsl", RES_DIR "/shaders/point_depth_frag.glsl");
	m_forwardCompositeShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/quad_frag.glsl");
	m_ssaoShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/ssao_frag.glsl");
	m_ssaoBlurShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/ssao_blur_frag.glsl");
	m_brightShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/bright_frag.glsl");
	m_finalCompositeShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/final_composite.glsl");

	// Initialize Background framebuffer
	m_backgroundFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	m_backgroundFB->createColorAttachment(); // color
	m_backgroundFB->addDepthTexture();
	m_backgroundFB->setDrawBuffers();
	if (!m_backgroundFB->isComplete()) {
		std::cerr << "Background framebuffer is incomplete" << std::endl;
	}

	// Initialize depth-only framebuffer for shadow mapping
	m_depthFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	// Create a depth texture
	GLuint shadowDepthTex;
	glGenTextures(1, &shadowDepthTex);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr); // TODO use variables for width and height
	// configure sampling and wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1,1,1,1 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach the depth texture to the framebuffer
	m_depthFB->setDepthTexture(shadowDepthTex, GL_DEPTH_ATTACHMENT, false);
	m_depthFB->bind();
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	m_depthFB->unbind();
	if (!m_depthFB->isComplete()) {
		std::cerr << "Depth framebuffer is incomplete" << std::endl;
	}

	// Point lights framebuffer
	m_pointShadowFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);

	// Create a depth texture
	GLuint pointShadowDepthTex;
	glGenTextures(1, &pointShadowDepthTex);
	glBindTexture(GL_TEXTURE_2D, pointShadowDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);// TODO use variables for width and height
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_pointShadowFB->setDepthTexture(pointShadowDepthTex, GL_DEPTH_ATTACHMENT, false);
	m_pointShadowFB->bind();
	// Disable writes to the color buffer
	glDrawBuffer(GL_NONE);
	// Disable reads from the color buffer
	glReadBuffer(GL_NONE);
	m_pointShadowFB->unbind();

	// Initialize Geometry pass framebuffers
	m_geometryFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	m_geometryFB->createColorAttachment(); // color
	m_geometryFB->createColorAttachment(); // normal
	m_geometryFB->createColorAttachment(); // position
	m_geometryFB->addDepthTexture();
	m_geometryFB->setDrawBuffers();
	if (!m_geometryFB->isComplete()) {
		std::cerr << "Geometry framebuffer is incomplete" << std::endl;
	}

	unsigned int SAMPLES = 16;
	m_geometryFB_MSAA = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	m_geometryFB_MSAA->createMultisampleColorAttachment(SAMPLES); // color
	m_geometryFB_MSAA->createMultisampleColorAttachment(SAMPLES); // normal
	m_geometryFB_MSAA->createMultisampleColorAttachment(SAMPLES); // position
	m_geometryFB_MSAA->addMultisampleDepthRenderBuffer(SAMPLES);
	m_geometryFB_MSAA->setDrawBuffers();
	if (!m_geometryFB_MSAA->isComplete()) {
		std::cerr << "Geometry MSAA framebuffer is incomplete" << std::endl;
	}
	

	// Initialize SSAO framebuffers
	m_ssaoFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	unsigned int ssaoColorBuffer;
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_renderTargetResolution.width, m_renderTargetResolution.height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_ssaoFB->addColorAttachment(ssaoColorBuffer); // SSAO color buffer
	m_ssaoFB->setDrawBuffers();
	if (!m_ssaoFB->isComplete()) {
		std::cerr << "SSAO framebuffer is incomplete" << std::endl;
	}

	// SSAO blur
	m_ssaoBlurFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	unsigned int ssaoBlurColorBuffer;
	glGenTextures(1, &ssaoBlurColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_renderTargetResolution.width, m_renderTargetResolution.height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_ssaoBlurFB->addColorAttachment(ssaoBlurColorBuffer); // SSAO blur color buffer
	m_ssaoBlurFB->setDrawBuffers();
	if (!m_ssaoBlurFB->isComplete()) {
		std::cerr << "SSAO blur framebuffer is incomplete" << std::endl;
	}

	// generate SSAO kernel
	ssaoKernel.reserve(64);
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f,
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f,
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		sample = glm::normalize(sample);
		sample *= static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		float scale = static_cast<float>(i) / 64.0f;
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// generate SSAO noise
	std::vector<glm::vec3> ssaoNoise;
	ssaoNoise.reserve(16);
	for (unsigned int i = 0; i < 16; ++i)
	{
		glm::vec3 noise(
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f,
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	// create SSAO noise texture
	glGenTextures(1, &m_ssaoNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, m_ssaoNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Bloom init
	m_bloomRenderer = std::make_unique<BloomRenderer>();
	m_bloomRenderer->init(m_renderTargetResolution.width, m_renderTargetResolution.height, 8);

	m_brightFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	m_brightFB->createColorAttachment(); // bright color
	m_brightFB->setDrawBuffers();
	if (!m_brightFB->isComplete()) {
		std::cerr << "Bright framebuffer is incomplete" << std::endl;
	}

	// Composite framebuffer (before bloom)
	m_hdrFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	m_hdrFB->createColorAttachment(); // color
	m_hdrFB->setDrawBuffers();
	if (!m_hdrFB->isComplete()) {
		std::cerr << "Final framebuffer is incomplete" << std::endl;
	}

	// Post process framebuffer
	m_finalCompositeFB = std::make_unique<Framebuffer>(m_renderTargetResolution.width, m_renderTargetResolution.height);
	m_finalCompositeFB->createColorAttachment(); // color
	m_finalCompositeFB->addDepthRenderBuffer();
	m_finalCompositeFB->setDrawBuffers();
	if (!m_finalCompositeFB->isComplete()) {
		std::cerr << "Post process framebuffer is incomplete" << std::endl;
	}

	// Skybox setup
	initSkybox();

	m_defaultWhiteTexture = createSolidTextureRGBA8(255, 255, 255, 255);
	m_defaultFlatNormalTexture = createSolidTextureRGBA8(128, 128, 255, 255);

	m_initialized = true;
}

SkyboxHandle Renderer::createSkyboxFromHDR(const std::string& path)
{
	GLuint hdr = loadHDRImage(path);

	GLEnvironment env = buildEnvironmentMaps(hdr);

	SkyboxHandle handle = m_skyboxes.size() + 1;
	m_skyboxes[handle] = env;
	return handle;
}

TextureHandle Renderer::createTextureFromFile(const std::string& path)
{
	GLTexture texture = loadTextureFromFile(path.c_str());

	TextureHandle handle = m_textures.size() + 1;
	m_textures[handle] = texture;
	return handle;
}

GLTexture Renderer::loadTextureFromFile(const char* path)
{
	GLTexture texture;
	TextureType type = TextureType::None;
	std::string filename = path;
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

	if (filename.find("albedo") != std::string::npos || filename.find("alb") != std::string::npos ||
		filename.find("diffuse") != std::string::npos || filename.find("color") != std::string::npos) {
		type = TextureType::ALBEDO;
	}
	else if (filename.find("normal") != std::string::npos || filename.find("norm") != std::string::npos ||
		filename.find("nrm") != std::string::npos) {
		type = TextureType::NORMAL;
	}
	else if (filename.find("metallic") != std::string::npos || filename.find("metal") != std::string::npos) {
		type = TextureType::METALLIC;
	}
	else if (filename.find("roughness") != std::string::npos || filename.find("rough") != std::string::npos) {
		type = TextureType::ROUGHNESS;
	}
	else if (filename.find("ao") != std::string::npos || filename.find("ambient") != std::string::npos ||
		filename.find("occlusion") != std::string::npos) {
		type = TextureType::AO;
	}


	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	float maxAniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(16.0f, maxAniso));

	// Load image data
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);  // Flip Y-axis during loading
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) {
		// Different handling based on texture type and channel count
		if (type == TextureType::ALBEDO) {
			// Color textures should use sRGB
			if (nrChannels == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else if (nrChannels == 4)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else {
			// Non-color textures should use linear space
			if (nrChannels == 1)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			else if (nrChannels == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else if (nrChannels == 4)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;
	}
	else {
		std::cout << "Failed to load texture: " << path << std::endl;
		glDeleteTextures(1, &texture.id);
		texture.id = 0;
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

void Renderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::updateLighting(const RenderDirectionalLight& light)
{
		m_pbrShader->setUniformVec3f("lightDir", light.lightDir);
		m_pbrShader->setUniform1f("ambientIntensity", light.ambientIntensity);
		m_pbrShader->setUniform1f("lightIntensity", light.lightIntensity);
}


void Renderer::render(const RenderData& renderData)
{
	FrameContext frame = buildFrameContext(renderData);

	// Background pass
	renderBackgroundPass(renderData, frame);

	// Depth pass
	renderShadowDepthPass(renderData, frame);

	// Point Light view depth pass
	renderPointShadowPass(renderData);

	// Geometry pass
	renderForwardGeometryPass(renderData, frame);

	// SSAO
	renderSSAOPass(renderData);

	// Composite pass
	renderForwardCompositePass();
	
	// Final composite pass
	renderFinalCompositePass(renderData);
}

FrameContext Renderer::buildFrameContext(const RenderData& renderData)
{
	FrameContext frame;
	frame.view = renderData.camera.viewMatrix;
	frame.projection = renderData.camera.projectionMatrix;
	frame.cameraPosition = renderData.camera.position;

	frame.lightDir = glm::normalize(renderData.directionalLight.lightDir);
	frame.lightSpaceMatrix = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 300.0f);
	glm::vec3 lightPos = frame.lightDir * 75.0f;
	frame.lightSpaceMatrix *= glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	if (renderData.skybox)
	{
		frame.environment = &m_skyboxes.at(renderData.skybox);
	}

	return frame;
}

void Renderer::renderBackgroundPass(const RenderData& renderData, const FrameContext& frame)
{
	m_backgroundFB->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw skybox
	glDepthMask(GL_FALSE);  // Don't write to depth buffer
	glDisable(GL_CULL_FACE);
	m_skyboxShader->bind();
	m_skyboxShader->setUniformMat4f("view", renderData.camera.viewMatrix);
	m_skyboxShader->setUniformMat4f("projection", renderData.camera.projectionMatrix);
	m_skyboxShader->setUniform1i("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	if (frame.environment)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, frame.environment->envCubemap);
	}
	glBindVertexArray(m_skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);  // Re-enable depth writing
	m_backgroundFB->unbind();

}

void Renderer::renderShadowDepthPass(const RenderData& renderData, const FrameContext& frame)
{
	m_depthFB->bind();
	glViewport(0, 0, 4096, 4096); // TODO use variables for width and height
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	m_depthShader->bind();
	m_depthShader->setUniformMat4f("lightSpaceMatrix", frame.lightSpaceMatrix);
	for (const auto& renderMesh : renderData.meshes)
	{
		m_depthShader->setUniformMat4f("model", renderMesh.transform);
		GLMesh& mesh = m_meshes.at(renderMesh.mesh);

		glBindVertexArray(mesh.vao);
		glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
	}
	m_depthFB->unbind();
	glViewport(0, 0, m_renderTargetResolution.width, m_renderTargetResolution.height); // reset viewport
}

void Renderer::renderForwardGeometryPass(const RenderData& renderData, const FrameContext& frame)
{
	if (useAA) 
	{
		m_geometryFB_MSAA->bind();
	}
	else 
	{
		m_geometryFB->bind();
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::vec3 camPos = renderData.camera.position;
	m_pbrShader->bind();
	m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z);
	m_pbrShader->setUniformMat4f("lightSpaceMatrix", frame.lightSpaceMatrix);
	m_pbrShader->setUniformMat4f("view", renderData.camera.viewMatrix);
	m_pbrShader->setUniformMat4f("projection", renderData.camera.projectionMatrix);
	m_pbrShader->setUniformVec3f("lightDir", frame.lightDir);
	m_pbrShader->setUniform1f("ambientIntensity", renderData.directionalLight.ambientIntensity);
	m_pbrShader->setUniform1f("lightIntensity", renderData.directionalLight.lightIntensity);
	m_pbrShader->setUniformVec3f("lightColor", renderData.directionalLight.color);
	glActiveTexture(GL_TEXTURE19);
	glBindTexture(GL_TEXTURE_2D, m_depthFB->depthTexture);
	m_pbrShader->setUniform1i("shadowMap", 19);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	if (frame.environment) {
		// IBL textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, frame.environment->irradianceMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, frame.environment->prefilterMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexture);

		// Set IBL uniforms
		m_pbrShader->bind();
		m_pbrShader->setUniform1i("irradianceMap", 0);
		m_pbrShader->setUniform1i("prefilterMap", 1);
		m_pbrShader->setUniform1i("brdfLUT", 2);
	}

	// Set point lights uniforms
	m_pbrShader->setUniform1i("numPointLights", renderData.lights.size());
	for (int i = 0; i < renderData.lights.size(); i++)
	{
		PointLightHandle handle = renderData.lights[i];
		const GLPointLight& pointLight = m_pointLights.at(handle);
		std::string idx = "pointLights[" + std::to_string(i) + "]";
		m_pbrShader->setUniformVec3f(idx + ".position", pointLight.position);
		m_pbrShader->setUniformVec3f(idx + ".color", pointLight.color);
		m_pbrShader->setUniform1f(idx + ".intensity", pointLight.intensity);
		m_pbrShader->setUniform1f(idx + ".constant", pointLight.constant);
		m_pbrShader->setUniform1f(idx + ".linear", pointLight.linear);
		m_pbrShader->setUniform1f(idx + ".quadratic", pointLight.quadratic);
		m_pbrShader->setUniform1f(idx + ".farPlane", pointLight.farPlane);

		glActiveTexture(GL_TEXTURE20 + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, pointLight.shadowCubemap);
		m_pbrShader->setUniform1i("pointShadowMaps[" + std::to_string(i) + "]", 20 + i);
	}

	for (const auto& renderMesh : renderData.meshes)
	{
		m_pbrShader->setUniformMat4f("model", renderMesh.transform);
		const GLMesh& mesh = m_meshes.at(renderMesh.mesh);
		const GLMaterial& material = m_materials.at(renderMesh.material);

		bindMaterial(*m_pbrShader, material);

		glBindVertexArray(mesh.vao);
		glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
	}

	if (useAA) {
		// Resolve MSAA FBO to single sample FBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_geometryFB_MSAA->fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_geometryFB->fbo);

		// Blit each color attachment
		for (unsigned int i = 0; i < m_geometryFB->textures.size(); ++i)
		{
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

			glBlitFramebuffer(0, 0, m_renderTargetResolution.width, m_renderTargetResolution.height,
				0, 0, m_renderTargetResolution.width, m_renderTargetResolution.height,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		// Restore draw buffers for single-sample FBO (needed when switching back to it)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_geometryFB->fbo);

			std::vector<GLenum> bufs(m_geometryFB->textures.size());
			for (size_t i = 0; i < bufs.size(); ++i) {
				bufs[i] = GL_COLOR_ATTACHMENT0 + (GLenum)i;
			}
			glDrawBuffers((GLsizei)bufs.size(), bufs.data());

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}

void Renderer::renderPointShadowPass(const RenderData& renderData)
{
	glDisable(GL_CULL_FACE);
	for (auto& renderLight : renderData.lights)
	{
		GLPointLight& light = m_pointLights.at(renderLight);

		m_pointShadowFB->bind();
		glViewport(0, 0, light.resolution, light.resolution);

		glm::vec3 lightPos = light.position;
		float nearPlane = 1.0f;
		float farPlane = light.farPlane;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

		m_pointDepthShader->bind();
		m_pointDepthShader->setUniformVec3f("lightPos", lightPos);
		m_pointDepthShader->setUniform1f("farPlane", farPlane);

		// render each face
		for (unsigned int face = 0; face < 6; ++face)
		{
			// Attach correct face
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				pointShadowDirection[face].CubemapFace, light.shadowCubemap, 0);

			glClear(GL_DEPTH_BUFFER_BIT);
			m_pointDepthShader->setUniformMat4f("shadowMatrix", shadowProj * glm::lookAt(lightPos, lightPos + pointShadowDirection[face].Target, pointShadowDirection[face].Up));

			for (const auto& renderMesh : renderData.meshes)
			{
				const GLMesh& mesh = m_meshes.at(renderMesh.mesh);
				m_pointDepthShader->setUniformMat4f("model", renderMesh.transform);

				glBindVertexArray(mesh.vao);
				glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
			}
		}

		m_pointShadowFB->unbind();
	}
	glViewport(0, 0, m_renderTargetResolution.width, m_renderTargetResolution.height);
	glEnable(GL_CULL_FACE);
}

void Renderer::renderSSAOPass(const RenderData& renderData)
{
	if (useSSAO) 
	{
		m_ssaoFB->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_ssaoShader->bind();
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, m_geometryFB->textures[2]); // position
		m_ssaoShader->setUniform1i("gPosition", 10);
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, m_geometryFB->textures[1]); // normal
		m_ssaoShader->setUniform1i("gNormal", 11);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, m_ssaoNoiseTexture);
		m_ssaoShader->setUniform1i("noiseTexture", 12);
		m_ssaoShader->setUniform3fv("samples", ssaoKernel, ssaoKernel.size());
		m_ssaoShader->setUniformMat4f("projection", renderData.camera.projectionMatrix);
		m_ssaoShader->setUniform2f("noiseScale", (float)m_renderTargetResolution.width / 4.0f, (float)m_renderTargetResolution.height / 4.0f);
		renderQuad();

		// SSAO blur pass to improve quality
		m_ssaoBlurFB->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_ssaoBlurShader->bind();
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, m_ssaoFB->textures[0]); // SSAO
		m_ssaoBlurShader->setUniform1i("ssaoTexture", 12);
		renderQuad();
	}
}

void Renderer::renderForwardCompositePass()
{
	m_hdrFB->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_forwardCompositeShader->bind();
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, m_geometryFB->textures[0]);
	m_forwardCompositeShader->setUniform1i("screenTexture", 13);
	if (useSSAO) {
		m_forwardCompositeShader->setUniform1i("useSSAO", 1);
	}
	else {
		m_forwardCompositeShader->setUniform1i("useSSAO", 0);
	}
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, m_ssaoBlurFB->textures[0]);
	m_forwardCompositeShader->setUniform1i("ssaoTexture", 14);
	renderQuad();
	m_forwardCompositeShader->unbind();
	m_hdrFB->unbind();
}

void Renderer::renderBloomPass()
{
	if (useBloom)
	{
		// Bright pass
		m_brightFB->bind();
		glClear(GL_COLOR_BUFFER_BIT);
		m_brightShader->bind();
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, m_hdrFB->textures[0]); // color
		m_brightShader->setUniform1i("sceneColor", 15);
		m_brightShader->setUniform1f("threshold", 1.0f);
		m_brightShader->setUniform1f("softThreshold", 0.85f);
		renderQuad();
		m_brightFB->unbind();

		// Bloom pass
		m_bloomRenderer->renderBloomTexture(m_brightFB->textures[0], bloomFilterRadius);
	}
}

void Renderer::renderFinalCompositePass(const RenderData& renderData)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_finalCompositeShader->bind();
	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_2D, m_hdrFB->textures[0]); // composite
	m_finalCompositeShader->setUniform1i("sceneTexture", 17);
	glActiveTexture(GL_TEXTURE18);
	glBindTexture(GL_TEXTURE_2D, m_backgroundFB->textures[0]); // background
	m_finalCompositeShader->setUniform1i("backgroundTexture", 18);
	m_finalCompositeShader->setUniform1f("exposure", renderData.directionalLight.exposure);
	m_finalCompositeShader->setUniform1i("toneMappingMode", 2);

	if (useBloom)
	{
		m_finalCompositeShader->setUniform1i("useBloom", 1);
		glActiveTexture(GL_TEXTURE16);
		glBindTexture(GL_TEXTURE_2D, m_bloomRenderer->bloomTexture()); // bloom
		m_finalCompositeShader->setUniform1i("bloomTexture", 16);
	}
	else {
		m_finalCompositeShader->setUniform1i("useBloom", 0);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderQuad();
	m_finalCompositeShader->unbind();
	glDisable(GL_BLEND);
	m_finalCompositeFB->unbind();
}

void Renderer::shutdown()
{
	m_initialized = false;
}


void Renderer::bindMaterial(const Shader& shader, const GLMaterial& material)
{
	shader.bind();

	shader.setUniformVec3f("material.albedo", material.albedo);
	shader.setUniform1f("material.metallic", material.metallic);
	shader.setUniform1f("material.roughness", material.roughness);
	shader.setUniform1f("material.ao", material.ao);
	shader.setUniformVec3f("material.emissiveColor", material.emissiveColor);

	glActiveTexture(GL_TEXTURE0 + Renderer::ALBEDO_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, material.albedoMap);
	shader.setUniform1i("material.albedoMap", Renderer::ALBEDO_TEXTURE_UNIT);

	glActiveTexture(GL_TEXTURE0 + Renderer::NORMAL_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, material.normalMap);
	shader.setUniform1i("material.normalMap", Renderer::NORMAL_TEXTURE_UNIT);

	glActiveTexture(GL_TEXTURE0 + Renderer::METAL_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, material.metallicMap);
	shader.setUniform1i("material.metallicMap", Renderer::METAL_TEXTURE_UNIT);

	glActiveTexture(GL_TEXTURE0 + Renderer::ROUGH_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, material.roughnessMap);
	shader.setUniform1i("material.roughnessMap", Renderer::ROUGH_TEXTURE_UNIT);

	glActiveTexture(GL_TEXTURE0 + Renderer::AO_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, material.aoMap);
	shader.setUniform1i("material.aoMap", Renderer::AO_TEXTURE_UNIT);

	glActiveTexture(GL_TEXTURE0 + Renderer::EMISSIVE_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, material.emissiveMap);
	shader.setUniform1i("material.emissiveMap", Renderer::EMISSIVE_TEXTURE_UNIT);
}

unsigned int Renderer::createSolidTextureRGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	unsigned int tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	uint8_t pixel[4] = { r, g, b, a };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

void Renderer::initSkybox()
{
	// Skybox init
	m_skyboxShader = std::make_unique<Shader>(RES_DIR"/shaders/skybox_vert.glsl", RES_DIR"/shaders/skybox_frag.glsl");
	m_equirectangularToCubemapShader = std::make_unique<Shader>(RES_DIR"/shaders/equirectangular_vert.glsl", RES_DIR"/shaders/equirectangular_frag.glsl");
	m_irradianceShader = std::make_unique<Shader>(RES_DIR"/shaders/equirectangular_vert.glsl", RES_DIR"/shaders/irradiance_frag.glsl");
	m_prefilterShader = std::make_unique<Shader>(RES_DIR"/shaders/prefilter_vert.glsl", RES_DIR"/shaders/prefilter_frag.glsl");
	m_brdfShader = std::make_unique<Shader>(RES_DIR"/shaders/brdf_vert.glsl", RES_DIR"/shaders/brdf_frag.glsl");

	// Setup skybox vao and vbo
	glGenVertexArrays(1, &m_skyboxVAO);
	glGenBuffers(1, &m_skyboxVBO);
	glBindVertexArray(m_skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RendererPrimitives::SkyboxCubeVertices), RendererPrimitives::SkyboxCubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);

	// Setup fbo
	glGenFramebuffers(1, &m_captureFBO);
	glGenRenderbuffers(1, &m_captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 2048, 2048);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Renderer::loadHDRImage(std::string path)
{
	unsigned int hdrTexture = 0;
	// Load radiance hdr map
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		if (nrChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		}
		else if (nrChannels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
		}
		else {
			std::cout << "Error: Unknown number of channels in hdr image" << std::endl;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image: " << path << std::endl;
		stbi_image_free(data);
	}
	return hdrTexture;
}

GLEnvironment Renderer::buildEnvironmentMaps(unsigned int hdrTexture)
{
	GLEnvironment environment{};
	environment.hdrTexture = hdrTexture;

	int maxMipLevels = 10;

	// Diffuse IBL
	// Create cubemap texture with hdr texture data
	glGenTextures(1, &environment.envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment.envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] = {
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	m_equirectangularToCubemapShader->bind();
	m_equirectangularToCubemapShader->setUniform1i("equirectangularMap", 0);
	m_equirectangularToCubemapShader->setUniformMat4f("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 2048, 2048);
	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		m_equirectangularToCubemapShader->setUniformMat4f("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment.envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Renderer::renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);

	glBindTexture(GL_TEXTURE_CUBE_MAP, environment.envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


	// Create texture for convoluted irradiance cubemap
	glGenTextures(1, &environment.irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment.irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// Create irradiance cubemap
	m_irradianceShader->bind();
	m_irradianceShader->setUniform1i("environmentMap", 0);
	m_irradianceShader->setUniformMat4f("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment.envCubemap);

	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		m_irradianceShader->setUniformMat4f("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment.irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Renderer::renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);

	// Specular IBL
	// Create texture for prefiltered environment map
	int specularSize = 512;
	glGenTextures(1, &environment.prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment.prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, specularSize, specularSize, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Generate mipmaps for the prefiltered environment map
	m_prefilterShader->bind();
	m_prefilterShader->setUniformMat4f("projection", captureProjection);
	m_prefilterShader->setUniform1i("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment.envCubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = specularSize * std::pow(0.5, mip);
		unsigned int mipHeight = specularSize * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);
		float roughness = (float)mip / (float)(maxMipLevels - 1);
		m_prefilterShader->setUniform1f("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			m_prefilterShader->setUniformMat4f("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment.prefilterMap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Renderer::renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);

	// Generate BRDF LUT texture
	glGenTextures(1, &m_brdfLUTTexture);
	glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	m_brdfShader->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Renderer::renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Reset viewport to window dimensions
	glViewport(0, 0, window_width, window_height);

	return environment;
}

PointLightHandle Renderer::createPointLight(PointLightData data)
{
	GLPointLight light;

	glGenTextures(1, &light.shadowCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, light.shadowCubemap);

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_DEPTH_COMPONENT32F,
			light.resolution, light.resolution,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	light.color = data.color;
	light.intensity = data.intensity;
	light.linear = data.linear;
	light.position = data.position;
	light.quadratic = data.quadratic;
	light.constant = data.constant;

	PointLightHandle handle = m_pointLights.size() + 1;
	m_pointLights[handle] = light;
	return handle;
}

void Renderer::setupMesh(GLMesh& mesh, MeshData& meshDesc)
{
	std::vector<Vertex>& verts = meshDesc.vertices;
	std::vector<unsigned int>& inds = meshDesc.indices;

	mesh.indexCount = inds.size();

	// Generate buffers
	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);

	glBindVertexArray(mesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), &inds[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
	glEnableVertexAttribArray(1);

	// Texture coordinates attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_texCoords));
	glEnableVertexAttribArray(2);

	// Tangent attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_tangent));
	glEnableVertexAttribArray(3);

	// Bitangent attribute
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_bitangent));
	glEnableVertexAttribArray(4);

	// Unbind the VAO
	glBindVertexArray(0);
}



MeshHandle Renderer::createMesh(MeshData meshDesc)
{
	GLMesh mesh{};
	setupMesh(mesh, meshDesc);

	MeshHandle handle = m_meshes.size() + 1;
	m_meshes[handle] = mesh;

	return handle;
}

MaterialHandle Renderer::createMaterial(MaterialData material)
{
	GLMaterial mat{};

	mat.albedo = material.albedo;
	mat.metallic = material.metallic;
	mat.roughness = material.roughness;
	mat.ao = material.ao;
	mat.emissiveColor = material.emissiveColor;

	auto getTextureId = [&](TextureHandle h, unsigned int fallback) {
		auto it = m_textures.find(h);
		return it != m_textures.end() ? it->second.id : fallback;
	};

	mat.albedoMap = getTextureId(material.albedoMap, m_defaultWhiteTexture);
	mat.metallicMap = getTextureId(material.metallicMap, m_defaultWhiteTexture);
	mat.roughnessMap = getTextureId(material.roughnessMap, m_defaultWhiteTexture);
	mat.aoMap = getTextureId(material.aoMap, m_defaultWhiteTexture);
	mat.emissiveMap = getTextureId(material.emissiveMap, m_defaultWhiteTexture);
	mat.normalMap = getTextureId(material.normalMap, m_defaultFlatNormalTexture);

	MaterialHandle handle = m_materials.size() + 1;
	m_materialData[handle] = material;
	m_materials[handle] = mat;

	return handle;
}

void Renderer::editMaterial(MaterialHandle handle, const MaterialData& data)
{
	auto it = m_materials.find(handle);
	if (it == m_materials.end())
		return;

	m_materialData[handle] = data;

	GLMaterial& material = it->second;
	material.albedo = data.albedo;
	material.metallic = data.metallic;
	material.roughness = data.roughness;
	material.ao = data.ao;
	material.emissiveColor = data.emissiveColor;
}

MaterialData Renderer::getMaterialData(MaterialHandle handle)
{
	auto it = m_materialData.find(handle);
	if (it != m_materialData.end())
		return it->second;

	return {};
}

void Renderer::editPointLight(PointLightHandle handle, const PointLightData& data)
{
	if (m_pointLights.find(handle) != m_pointLights.end())
	{
		GLPointLight& pointLight = m_pointLights.at(handle);
		pointLight.position = data.position;
		pointLight.color = data.color;
		pointLight.intensity = data.intensity;
		pointLight.constant = data.constant;
		pointLight.quadratic = data.quadratic;
		pointLight.linear = data.linear;
	}
}

PointLightData Renderer::getPointLightData(PointLightHandle handle)
{
	PointLightData data;

	if (m_pointLights.find(handle) != m_pointLights.end())
	{
		GLPointLight& pointLight = m_pointLights.at(handle);
		data.position = pointLight.position;
		data.color = pointLight.color;
		data.intensity = pointLight.intensity;
		data.constant = pointLight.constant;
		data.quadratic = pointLight.quadratic;
		data.linear = pointLight.linear;
	}

	return data;
}

BloomRenderer::BloomRenderer()
{
}

BloomRenderer::~BloomRenderer()
{
	destroy();
}

bool BloomRenderer::init(unsigned int windowWidth, unsigned int windowHeight, unsigned int numMips)
{
	if (m_init)
	{
		return true;
	}

	glGenFramebuffers(1, &m_bloomFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFBO);

	glm::vec2 mipSize((float)windowWidth, (float)windowHeight);
	glm::ivec2 mipIntSize((int)windowWidth, (int)windowHeight);

	for (unsigned int i = 0; i < numMips; i++)
	{
		BloomMip mip;
		mipSize *= 0.5f;
		mipIntSize = glm::max(mipIntSize / 2, glm::ivec2(1));
		mip.size = glm::vec2(mipIntSize);
		mip.size = mipSize;
		mip.intSize = mipIntSize;

		glGenTextures(1, &mip.texture);
		glBindTexture(GL_TEXTURE_2D, mip.texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
			(int)mipSize.x, (int)mipSize.y,
			0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		m_mipChain.emplace_back(mip);
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, m_mipChain[0].texture, 0);

	// Create a depth buffer for the bloom FBO
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	// Store the depth buffer handle as a member variable
	m_depthBuffer = depthBuffer;

	// set draw buffers
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	// check completion status
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Bloom framebuffer is incomplete: " << status << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_downsampleShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/bloom_downsample.glsl");
	m_downsampleShader->setUniform1i("srcTexture", 0);
	m_downsampleShader->unbind();

	m_upsampleShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/bloom_upsample.glsl");
	m_upsampleShader->setUniform1i("srcTexture", 0);
	m_upsampleShader->unbind();


	m_init = true;
	return true;
}

void BloomRenderer::destroy()
{
	if (m_init)
	{
		glDeleteFramebuffers(1, &m_bloomFBO);
		glDeleteRenderbuffers(1, &m_depthBuffer);
		for (auto& mip : m_mipChain)
		{
			glDeleteTextures(1, &mip.texture);
		}
		m_mipChain.clear();
		m_init = false;
	}
}

void BloomRenderer::renderBloomTexture(unsigned int srcTexture, float filterRadius)
{
	if (!m_init)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderDownsamples(srcTexture);

	renderUpsamples(filterRadius);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Restore viewport
	glViewport(0, 0, window_width, window_height);
}

unsigned int BloomRenderer::bloomTexture()
{
	return m_mipChain[0].texture;
}

void BloomRenderer::renderDownsamples(unsigned int srcTexture)
{
	m_downsampleShader->bind();
	m_downsampleShader->setUniform2f("srcResolution", m_srcViewportSizeFloat.x, m_srcViewportSizeFloat.y);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, srcTexture);
	for (unsigned int i = 1; i < m_mipChain.size(); i++)
	{
		const BloomMip& mip = m_mipChain[i];
		glViewport(0, 0, mip.size.x, mip.size.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);
		Renderer::renderQuad();
		m_downsampleShader->setUniform2f("srcResolution", mip.size.x, mip.size.y);
		glBindTexture(GL_TEXTURE_2D, mip.texture);
	}
	m_downsampleShader->unbind();
}

void BloomRenderer::renderUpsamples(float filterRadius)
{
	m_upsampleShader->bind();
	m_upsampleShader->setUniform1f("filterRadius", filterRadius);

	// Enable additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	for (int i = m_mipChain.size() - 1; i > 0; i--)
	{
		const BloomMip& mip = m_mipChain[i];
		const BloomMip& nextMip = m_mipChain[i - 1];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mip.texture);

		glViewport(0, 0, nextMip.size.x, nextMip.size.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture, 0);

		Renderer::renderQuad();
	}

	// Disable additive blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	m_upsampleShader->unbind();
}


