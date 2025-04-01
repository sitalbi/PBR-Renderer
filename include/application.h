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

	float m_deltaTime = 0.0f;
	float m_lastFrame = 0.0f;
	float m_currentFrame = 0.0f;

	std::shared_ptr<Shader> m_basicShader;

	std::unordered_map<MeshType, std::shared_ptr<Mesh>> m_meshes;

	std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;

	
	std::vector<const char*> m_meshTypes;
	int m_meshTypeIndex = 0;

	void initUI();
	void updateUI();
	void processInput(float deltaTime);
	void setCallbacks();

	void deltaTime();
};