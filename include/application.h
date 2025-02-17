#pragma once

#include "renderer.h"
#include "camera.h"


class Application
{
public:
	Application();
	~Application();

	void run();
	void init();
	void shutdown();
private:
	std::unique_ptr<Renderer> m_renderer;
	Camera m_camera;

	double m_lastX, m_lastY;
	bool m_firstMouse = true;

	std::shared_ptr<Shader> m_basicShader;

	std::shared_ptr<Mesh> m_basicMesh;

	void processInput();
	void setCallbacks();
};