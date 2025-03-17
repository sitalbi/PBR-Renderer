#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <vector>
#include "entity.h"
#include "camera.h"
#include "scene.h"

#define window_width 1920
#define window_height 1080

class Renderer
{
public:
	Renderer();
	~Renderer();

	void init();
	void update();
	void shutdown();

	void setCamera(Camera* camera) { m_camera = camera; }
	Camera* getCamera() { return m_camera; }
	void setLightDir(const glm::vec3& lightDir) { m_lightDir = lightDir; }
	void setLightColor(const glm::vec3& lightColor) { m_lightColor = lightColor; }

	void setCurrentScene(std::unique_ptr<Scene> scene) { m_currentScene = std::move(scene); }
	std::unique_ptr<Scene>& getCurrentScene() { return m_currentScene; }

	std::shared_ptr<Shader> getBasicShader() { return m_basicShader; }
	std::shared_ptr<Shader> getPBRShader() { return m_pbrShader; }

	GLFWwindow* getWindow() { return m_window; }

private:
	GLFWwindow* m_window;

	Camera* m_camera;

	glm::vec3 m_lightDir = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	bool m_initialized = false;

	std::unique_ptr<Scene> m_currentScene;

	std::shared_ptr<Shader> m_basicShader;
	std::shared_ptr<Shader> m_pbrShader;

	void clear();
	void render();
	void swapBuffers();

	void initUI();
	void renderUI();
};
