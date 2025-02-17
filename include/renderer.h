#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <vector>
#include "entity.h"
#include "camera.h"

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

	void addEntity(std::shared_ptr<Entity> entity) { m_entities.push_back(entity); }

	void setCamera(Camera* camera) { m_camera = camera; }

	std::shared_ptr<Shader> getBasicShader() { return m_basicShader; }

	GLFWwindow* getWindow() { return m_window; }

private:
	GLFWwindow* m_window;

	Camera* m_camera;

	bool m_initialized = false;

	std::vector<std::shared_ptr<Entity>> m_entities;

	std::shared_ptr<Shader> m_basicShader;

	void clear();
	void render();
	void swapBuffers();
};
