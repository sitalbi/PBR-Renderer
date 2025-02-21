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

	std::shared_ptr<Mesh> getMesh(MeshType type) { return m_meshes[type]; }

private:
	std::unique_ptr<Renderer> m_renderer;
	Camera m_camera;

	double m_lastX, m_lastY;
	bool m_firstMouse = true;

	std::shared_ptr<Shader> m_basicShader;

	std::unordered_map<MeshType, std::shared_ptr<Mesh>> m_meshes;

	std::shared_ptr<Material> m_basicMaterial;

	void initUI();
	void updateUI();
	void processInput();
	void setCallbacks();
};