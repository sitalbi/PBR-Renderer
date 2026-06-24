#pragma once

#include <Scene.h>
#include "Camera.h"
#include <renderer/Renderer.h>
#include <renderer/RenderData.h>
#include <renderer/Shader.h>


class Application
{
public:
	Application();
	~Application();

	void run();
	void init();
	void shutdown();

private:

	GLFWwindow* m_window;

	Renderer m_renderer;
	Camera m_camera;

	std::unordered_map<int, bool> m_keyStates;

	double m_lastX, m_lastY;
	bool m_firstMouse = true;
	bool m_dockInitialized = false;

	bool m_showEditorUI = true;

	float m_deltaTime = 0.0f;
	float m_lastFrame = 0.0f;
	float m_currentFrame = 0.0f;

	std::shared_ptr<Shader> m_basicShader;

	Scene m_currentScene;

	std::shared_ptr<Model> m_sphereModel;
	std::shared_ptr<Model> m_cubeModel;

	MaterialHandle m_defaultMaterial = 0;

	Entity* selectedEntity = nullptr;
	
	std::vector<const char*> m_meshTypes;
	int m_meshTypeIndex = 0;

	void initUI();
	void updateUI();
	void update(float deltaTime);
	void processInput(float deltaTime);
	void setCallbacks();

	RenderData buildRenderData();

	void deltaTime();

	void setupImGuiStyle();

	void onPressedKey(int key, const std::function<void()>& callback);

	void updateGizmo();

	void setDockingSpace();
};
