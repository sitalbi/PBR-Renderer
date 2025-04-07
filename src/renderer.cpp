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

	// Enable MSAA
	glEnable(GL_MULTISAMPLE);

	// OpenGL settings
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	glDisable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	// Skybox settings
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	m_pbrShader = std::make_shared<Shader>(RES_DIR "/shaders/basic_vert.glsl", RES_DIR  "/shaders/pbr_frag.glsl");
	if (m_pbrShader)
	{
		m_pbrShader->bind();
		m_pbrShader->setUniform3f("lightDir", m_lightDir.x, m_lightDir.y, m_lightDir.z);
		m_pbrShader->setUniform3f("lightColor", m_lightColor.x, m_lightColor.y, m_lightColor.z);
		glm::vec3 camPos = m_camera->getPosition();
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z);
	}

	m_compositeShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/quad_frag.glsl");

	m_ssaoShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/ssao_frag.glsl");

	m_ssaoBlurShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/ssao_blur_frag.glsl");

	// Initialize Geometry pass framebuffer
	m_gBuffer = std::make_unique<Framebuffer>(window_width, window_height);
	m_gBuffer->createColorAttachment(); // color
	m_gBuffer->createColorAttachment(); // normal
	m_gBuffer->createColorAttachment(); // position
	m_gBuffer->addDepthTexture();
	m_gBuffer->setDrawBuffers();
	if (!m_gBuffer->isComplete()) {
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

	// Final framebuffer
	m_finalFB = std::make_unique<Framebuffer>(window_width, window_height);
	m_finalFB->createColorAttachment(); // color
	m_finalFB->setDrawBuffers();
	if (!m_finalFB->isComplete()) {
		std::cerr << "Final framebuffer is incomplete" << std::endl;
	}

	m_initialized = true;
}

void Renderer::clear()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render()
{
	// Geometry pass into G-Buffer
	if (m_currentScene)
	{
		m_gBuffer->bind();
		glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::vec3 camPos = m_camera->getPosition();
		m_pbrShader->bind();
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z); 
		m_currentScene->draw(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
	}

	// SSAO pass
	if (useSSAO) {
		m_ssaoFB->bind();
		glClear(GL_COLOR_BUFFER_BIT);
		m_ssaoShader->bind();
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->textures[2]); // position
		m_ssaoShader->setUniform1i("gPosition", 10);
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->textures[1]); // normal
		m_ssaoShader->setUniform1i("gNormal", 11);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, m_ssaoNoiseTexture);
		m_ssaoShader->setUniform1i("noiseTexture", 12);
		m_ssaoShader->setUniform3fv("samples", ssaoKernel, ssaoKernel.size());
		m_ssaoShader->setUniformMat4f("projection", m_camera->getProjectionMatrix());
		renderQuad();

		// SSAO blur pass to improve quality
		m_ssaoBlurFB->bind();
		glClear(GL_COLOR_BUFFER_BIT);
		m_ssaoBlurShader->bind();
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, m_ssaoFB->textures[0]); // SSAO
		m_ssaoBlurShader->setUniform1i("ssaoTexture", 12);
		renderQuad();
	}

	// Final pass (create composite image by blending G-Buffer and other effects)
	m_finalFB->bind();
	clear();
	m_compositeShader->bind();
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer->textures[0]);
	m_compositeShader->setUniform1i("screenTexture", 13);
	if (useSSAO) {
		m_compositeShader->setUniform1i("useSSAO", 1);
	}
	else {
		m_compositeShader->setUniform1i("useSSAO", 0);
	}
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, m_ssaoBlurFB->textures[0]);
	m_compositeShader->setUniform1i("ssaoTexture", 14);
	renderQuad();
	m_compositeShader->unbind();
	m_finalFB->unbind();
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

	ImGui::Image((ImTextureID)(intptr_t)m_finalFB->textures[0],
		viewportSize,
		ImVec2(uMin, vMax), 
		ImVec2(uMax, vMin));
	ImGui::End();

	// Render UI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
