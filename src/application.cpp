#include "application.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

Application::Application()
{
	m_renderer = std::make_unique<Renderer>();
	m_camera = Camera(window_width, window_height, glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f);
	m_lastX = 0;
	m_lastY = 0;
	m_firstMouse = true;
}

Application::~Application()
{
}

void Application::run()
{
	while (!glfwWindowShouldClose(m_renderer->getWindow()))
	{
		updateUI();
		m_renderer->update();
		glfwPollEvents();

		processInput();
	}
	shutdown();
}

void Application::init()
{
	m_renderer->setLightPos(glm::vec3(0.0f, 2.0f, -4.5f));
	m_renderer->setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
	m_renderer->setCamera(&m_camera);
	m_renderer->init();

	initUI();

	setCallbacks();

	// Create cube and sphere meshes
	std::shared_ptr<Mesh> m_cubeMesh = std::make_shared<Mesh>();
	m_cubeMesh->loadCube();
	std::shared_ptr<Mesh> m_sphereMesh = std::make_shared<Mesh>();
	m_sphereMesh->loadSphere(1.0f, 50);

	m_meshes[MeshType::CUBE] = m_cubeMesh;
	m_meshes[MeshType::SPHERE] = m_sphereMesh;

	Material m_basicMaterial;
	m_basicMaterial.shader = m_renderer->getPBRShader();
	m_basicMaterial.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
	m_basicMaterial.metallic = 1.0f;
	m_basicMaterial.roughness = 0.1f;
	m_basicMaterial.ao = 0.1f;

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();

	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::CUBE], m_basicMaterial, glm::vec3(2.0f, 0.0f, 0.0f)));
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_basicMaterial, glm::vec3(-2.0f, 0.0f, 0.0f)));

	m_renderer->setCurrentScene(std::move(scene));

}

void Application::shutdown()
{
	m_renderer->shutdown();
}

void Application::initUI()
{
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_renderer->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void Application::updateUI()
{
	std::unique_ptr<Scene>& currentScene = m_renderer->getCurrentScene();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Information Panel
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, 50), ImGuiCond_Always);

	ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("App average %.3f ms/frame (%.1f FPS)\n", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	// Scene Editor panel
	ImGui::SetNextWindowPos(ImVec2(10, 100), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(0, 400), ImGuiCond_Always);

	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	for (auto& entity : currentScene->getEntities()) {
		if (ImGui::TreeNode(entity->getName().c_str())) {
			if (ImGui::TreeNode("Material")) {
				Material& material = entity->getMaterial();
				ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
				ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);
				ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
				ImGui::SliderFloat("Ambient Occlusion", &material.ao, 0.0f, 1.0f);

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Transform")) {
				ImGui::InputFloat3("Position", glm::value_ptr(entity->position));
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}

	// Add Entity 
	bool& isAddingEntity = currentScene->getIsAddingEntity();
	glm::vec3& newPosition = currentScene->getNewEntityPosition();

	if (ImGui::Button("Add Entity")) {
		isAddingEntity = true;  
	}

	if (isAddingEntity) {
		ImGui::InputFloat3("Position", glm::value_ptr(newPosition));

		if (ImGui::Button("Confirm Entity")) {
			Material material;
			material.shader = m_renderer->getPBRShader();
			material.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
			material.metallic = 1.0f;
			material.roughness = 0.1f;
			material.ao = 0.1f;

			currentScene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], material, newPosition));
			isAddingEntity = false;  
			newPosition = glm::vec3(0.0f); 
		}

		if (ImGui::Button("Cancel")) {
			isAddingEntity = false;
			newPosition = glm::vec3(0.0f); 
		}
	}

	ImGui::End();
}

void Application::processInput()
{
	if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(m_renderer->getWindow(), true); 
	}
	
	// TODO: refactor using callbacks
	double xpos, ypos;
	glfwGetCursorPos(m_renderer->getWindow(), &xpos, &ypos);
	static double lastX = xpos;
	static double lastY = ypos;
	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	if (glfwGetMouseButton(m_renderer->getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		Camera* cam = m_renderer->getCamera();
		cam->orbit(xoffset, yoffset);

		// Hide cursor
		glfwSetInputMode(m_renderer->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} 
	else if (glfwGetMouseButton(m_renderer->getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		// Show cursor
		glfwSetInputMode(m_renderer->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Application::setCallbacks()
{
	GLFWwindow* window = m_renderer->getWindow();

	glfwSetWindowUserPointer(window, &m_camera);

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
		cam->zoom(yoffset);
	});
}

