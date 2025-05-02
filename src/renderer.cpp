#include "renderer.h"
#include <iostream>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>

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

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_SAMPLES, 8);
	m_window = glfwCreateWindow(window_width, window_height, "Renderer", NULL, NULL);
	if (!m_window) {
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(m_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return;
	}

	// Enable anti-aliasing
	glEnable(GL_MULTISAMPLE);

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
		m_pbrShader->setUniform3f("lightDir", lightDir.x, lightDir.y, lightDir.z);
		m_pbrShader->setUniform3f("lightColor", m_lightColor.x, m_lightColor.y, m_lightColor.z);
		glm::vec3 camPos = m_camera->getPosition();
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z);
	}

	m_depthShader = std::make_shared<Shader>(RES_DIR "/shaders/depth_vert.glsl", RES_DIR "/shaders/empty_frag.glsl");
	m_lightingShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/quad_frag.glsl");
	m_ssaoShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/ssao_frag.glsl");
	m_ssaoBlurShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/ssao_blur_frag.glsl");
	m_brightShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/bright_frag.glsl");
	m_finalCompoShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/final_composite.glsl");

	// Initialize Background framebuffer
	m_backgroundFB = std::make_unique<Framebuffer>(window_width, window_height);
	m_backgroundFB->createColorAttachment(); // color
	m_backgroundFB->addDepthTexture();
	m_backgroundFB->setDrawBuffers();
	if (!m_backgroundFB->isComplete()) {
		std::cerr << "Background framebuffer is incomplete" << std::endl;
	}

	// Initialize depth-only framebuffer for shadow mapping
	m_depthFB = std::make_unique<Framebuffer>(window_width, window_height);
	// Create a depth texture
	GLuint shadowDepthTex;
	glGenTextures(1, &shadowDepthTex);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr); // TODO use variables for width and height
	// configure sampling and wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1,1,1,1 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach the depth texture to the framebuffer
	m_depthFB->setDepthTexture(shadowDepthTex, GL_DEPTH_ATTACHMENT);
	m_depthFB->bind();
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	m_depthFB->unbind();
	if (!m_depthFB->isComplete()) {
		std::cerr << "Depth framebuffer is incomplete" << std::endl;
	}

	// Initialize Geometry pass framebuffer
	m_geometryFB = std::make_unique<Framebuffer>(window_width, window_height);
	m_geometryFB->createColorAttachment(); // color
	m_geometryFB->createColorAttachment(); // normal
	m_geometryFB->createColorAttachment(); // position
	m_geometryFB->addDepthTexture();
	m_geometryFB->setDrawBuffers();
	if (!m_geometryFB->isComplete()) {
		std::cerr << "Geometry framebuffer is incomplete" << std::endl;
	}

	// Initialize SSAO framebuffers
	m_ssaoFB = std::make_unique<Framebuffer>(window_width, window_height);
	unsigned int ssaoColorBuffer;
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_ssaoFB->addColorAttachment(ssaoColorBuffer); // SSAO color buffer
	m_ssaoFB->setDrawBuffers();
	if (!m_ssaoFB->isComplete()) {
		std::cerr << "SSAO framebuffer is incomplete" << std::endl;
	}

	// SSAO blur
	m_ssaoBlurFB = std::make_unique<Framebuffer>(window_width, window_height);
	unsigned int ssaoBlurColorBuffer;
	glGenTextures(1, &ssaoBlurColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
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
	m_bloomRenderer->init(window_width, window_height, 10);

	m_brightFB = std::make_unique<Framebuffer>(window_width, window_height);
	m_brightFB->createColorAttachment(); // bright color
	m_brightFB->setDrawBuffers();
	if (!m_brightFB->isComplete()) {
		std::cerr << "Bright framebuffer is incomplete" << std::endl;
	}

	// Composite framebuffer (before bloom)
	m_hdrFB = std::make_unique<Framebuffer>(window_width, window_height);
	m_hdrFB->createColorAttachment(); // color
	m_hdrFB->setDrawBuffers();
	if (!m_hdrFB->isComplete()) {
		std::cerr << "Final framebuffer is incomplete" << std::endl;
	}

	// Post process framebuffer
	m_finalCompositeFB = std::make_unique<Framebuffer>(window_width, window_height);
	m_finalCompositeFB->createColorAttachment(); // color
	m_finalCompositeFB->addDepthRenderBuffer();
	m_finalCompositeFB->setDrawBuffers();
	if (!m_finalCompositeFB->isComplete()) {
		std::cerr << "Post process framebuffer is incomplete" << std::endl;
	}

	m_initialized = true;
}

void Renderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::updateLighting()
{
	if (m_pbrShader)
	{
		m_pbrShader->bind();
		m_pbrShader->setUniform3f("lightDir", lightDir.x, lightDir.y, lightDir.z);
		m_pbrShader->unbind();
	}
}


void Renderer::render()
{
	// Background pass
	m_backgroundFB->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_backgroundFB->bind();
	m_currentScene->drawSkybox(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
	m_backgroundFB->unbind();

	// Depth pass
	m_depthFB->bind();
	glViewport(0, 0, 2048, 2048); // TODO use variables for width and height
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	// light space matrix
	glm::mat4 lightSpaceMatrix = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 75.0f);
	glm::vec3 lightPos = lightDir*20.0f;
	lightSpaceMatrix *= glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_depthShader->bind();
	m_depthShader->setUniformMat4f("lightSpaceMatrix", lightSpaceMatrix);
	for (auto& entity : m_currentScene->getEntities())
	{
		m_depthShader->setUniformMat4f("model", entity->getModelMatrix());
		entity->drawMesh();
	}
	m_depthFB->unbind();
	glViewport(0, 0, window_width, window_height); // reset viewport
	
	// Geometry pass
	if (m_currentScene)
	{
		m_geometryFB->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::vec3 camPos = m_camera->getPosition();
		m_pbrShader->bind();
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z); 
		m_pbrShader->setUniformMat4f("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE19);
		glBindTexture(GL_TEXTURE_2D, m_depthFB->depthTexture);
		m_pbrShader->setUniform1i("shadowMap", 19);
		m_currentScene->draw(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
	}

	// SSAO
	if (useSSAO) {
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
		m_ssaoShader->setUniformMat4f("projection", m_camera->getProjectionMatrix());
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

	// Lighting pass (SSAO, tone mapping)
	m_hdrFB->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_lightingShader->bind();
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, m_geometryFB->textures[0]);
	m_lightingShader->setUniform1i("screenTexture", 13);
	if (useSSAO) {
		m_lightingShader->setUniform1i("useSSAO", 1);
	}
	else {
		m_lightingShader->setUniform1i("useSSAO", 0);
	}
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, m_ssaoBlurFB->textures[0]);
	m_lightingShader->setUniform1i("ssaoTexture", 14);
	renderQuad();
	m_lightingShader->unbind();
	m_hdrFB->unbind();

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
		m_brightShader->setUniform1f("softThreshold", 0.95f);
		renderQuad();
		m_brightFB->unbind();

		// Bloom pass
		m_bloomRenderer->renderBloomTexture(m_brightFB->textures[0], 0.0015f);
	}
	

	// Final composite pass
	m_finalCompositeFB->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_finalCompoShader->bind();
	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_2D, m_hdrFB->textures[0]); // composite
	m_finalCompoShader->setUniform1i("sceneTexture", 17);
	glActiveTexture(GL_TEXTURE18);
	glBindTexture(GL_TEXTURE_2D, m_backgroundFB->textures[0]); // background
	m_finalCompoShader->setUniform1i("backgroundTexture", 18);
	m_finalCompoShader->setUniform1f("exposure",exposure);

	if (useBloom)
	{
		m_finalCompoShader->setUniform1i("useBloom", 1);
		glActiveTexture(GL_TEXTURE16);
		glBindTexture(GL_TEXTURE_2D, m_bloomRenderer->bloomTexture()); // bloom
		m_finalCompoShader->setUniform1i("bloomTexture", 16);
	}
	else {
		m_finalCompoShader->setUniform1i("useBloom", 0);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderQuad();
	m_finalCompoShader->unbind();
	glDisable(GL_BLEND);
	m_finalCompositeFB->unbind();
}

void Renderer::update()
{
	clear();
	render();
	renderUI();
	swapBuffers();
}

void Renderer::shutdown()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
	m_initialized = false;
}

void Renderer::swapBuffers()
{
	glfwSwapBuffers(m_window);
}

void Renderer::renderUI()
{
	// Render Viewport
	ImGui::Begin("Viewport");
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	// Crop the viewport to a 16:9 aspect ratio
	float aspectRatio = (float) window_width / (float)window_height;
	float viewportAspectRatio = viewportSize.x / viewportSize.y;

	float uMin = 0.0f;
	float vMin = 0.0f;
	float uMax = 1.0f;
	float vMax = 1.0f;

	if (viewportAspectRatio > aspectRatio) {
		float scale = viewportSize.x / (viewportSize.y * aspectRatio);
		float vCrop = (1.0f - 1.0f / scale) * 0.5f;
		vMin = vCrop;
		vMax = 1.0f - vCrop;
	}
	else if (viewportAspectRatio <= aspectRatio) {
		float scale = (viewportSize.y * aspectRatio) / viewportSize.x;
		float uCrop = (1.0f - 1.0f / scale) * 0.5f;
		uMin = uCrop;
		uMax = 1.0f - uCrop;
	}

	ImGui::Image((ImTextureID)(intptr_t)m_finalCompositeFB->textures[0],
		viewportSize,
		ImVec2(uMin, vMax), 
		ImVec2(uMax, vMin));
	ImGui::End();

	// Render UI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
		mipIntSize /= 2;
		mip.size = mipSize;
		mip.intSize = mipIntSize;

		glGenTextures(1, &mip.texture);
		glBindTexture(GL_TEXTURE_2D, mip.texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
			(int)mipSize.x, (int)mipSize.y,
			0, GL_RGB, GL_FLOAT, nullptr);
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, mip.texture, 0);
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, nextMip.texture, 0);

		Renderer::renderQuad();
	}

	// Disable additive blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	m_upsampleShader->unbind();
}


