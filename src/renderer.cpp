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

	m_quadShader = std::make_unique<Shader>(RES_DIR "/shaders/quad_vert.glsl", RES_DIR "/shaders/quad_frag.glsl");

	// Initialize Geometry pass framebuffer
	m_geometryFB = std::make_unique<Framebuffer>(window_width, window_height);
	m_geometryFB->createColorAttachment(); // color
	m_geometryFB->createColorAttachment(); // normal
	m_geometryFB->addDepthAttachment();
	m_geometryFB->setDrawBuffers();
	if (!m_geometryFB->isComplete()) {
		std::cerr << "Geometry framebuffer is incomplete" << std::endl;
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
	// Geometry pass
	if (m_currentScene)
	{
		m_geometryFB->bind();
		clear();
		glm::vec3 camPos = m_camera->getPosition();
		m_pbrShader->bind();
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z); 
		m_currentScene->draw(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
	}

	// Final pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_quadShader->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_geometryFB->textures[0]);
	m_quadShader->setUniform1i("screenTexture", 0);
	renderQuad();
	m_quadShader->unbind();
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

	ImGui::Image((ImTextureID)(intptr_t)m_geometryFB->textures[0],
		viewportSize,
		ImVec2(uMin, vMax), 
		ImVec2(uMax, vMin));
	ImGui::End();

	// Render UI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
