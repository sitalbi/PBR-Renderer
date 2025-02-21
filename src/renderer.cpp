#include "renderer.h"
#include <iostream>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>

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

	// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	m_pbrShader = std::make_shared<Shader>(RES_DIR "/shaders/basic_vert.glsl", RES_DIR  "/shaders/pbr_frag.glsl");
	if (m_pbrShader)
	{
		m_pbrShader->bind();
		m_pbrShader->setUniform3f("lightPos", m_lightPos.x, m_lightPos.y, m_lightPos.z);
		m_pbrShader->setUniform3f("lightColor", m_lightColor.x, m_lightColor.y, m_lightColor.z);
		glm::vec3 camPos = m_camera->getPosition();
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z);
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
	if (m_currentScene)
	{
		glm::vec3 camPos = m_camera->getPosition();
		m_pbrShader->setUniform3f("camPos", camPos.x, camPos.y, camPos.z);
		m_currentScene->draw(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
	}
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
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
