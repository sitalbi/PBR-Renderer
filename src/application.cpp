#include "application.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>
#include <future>
#include <magic_enum.hpp>

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
		deltaTime();

		updateUI();
		m_renderer->update();
		glfwPollEvents();

		processInput(m_deltaTime);
	}
	shutdown();
}

void Application::init()
{
	m_renderer->setLightDir(glm::vec3(0.0f, 1.0f, -1.0f));
	m_renderer->setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
	m_renderer->setCamera(&m_camera);
	m_renderer->init();

	initUI();

	setCallbacks();

	// Create default meshes
	std::shared_ptr<Mesh> m_cubeMesh = std::make_shared<Mesh>();
	m_cubeMesh->loadCube();
	std::shared_ptr<Mesh> m_sphereMesh = std::make_shared<Mesh>();
	m_sphereMesh->loadSphere(1.0f, 50);


	std::shared_ptr<Mesh> m_suzanneMesh = std::make_shared<Mesh>();

	m_suzanneMesh->loadModel(RES_DIR"/models/suzanne.obj");
	m_meshes[MeshType::SUZANNE] = m_suzanneMesh;

	m_meshes[MeshType::CUBE] = m_cubeMesh;
	m_meshes[MeshType::SPHERE] = m_sphereMesh;

	// Create default material
	Material m_basicMaterial;
	m_basicMaterial.shader = m_renderer->getPBRShader();
	m_basicMaterial.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
	m_basicMaterial.metallic = 0.01f;
	m_basicMaterial.roughness = 0.9f;
	m_basicMaterial.ao = 0.25f;

	// Create textured material
	Material m_texturedMaterial;
	m_texturedMaterial.shader = m_renderer->getPBRShader();

	// TODO: load materials from folder and create material class automatically by reading the existing files
	m_texturedMaterial.albedoMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_albedo.png");
	m_texturedMaterial.normalMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_normal-ogl.png");
	m_texturedMaterial.metallicMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_metallic.png");
	m_texturedMaterial.roughnessMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_roughness.png");
	//m_texturedMaterial.aoMap = std::make_shared<Texture>(RES_DIR"/textures/materials/scuffed-plastic-ao.png");
	m_texturedMaterial.ao = 0.5f;

	m_texturedMaterial.useAlbedoMap = true;
	m_texturedMaterial.useNormalMap = true;
	m_texturedMaterial.useMetalMap = true;
	m_texturedMaterial.useRoughMap = true;
	//m_texturedMaterial.useAoMap = true;

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_basicMaterial, glm::vec3(-5.0f, 0.0f, 0.0f)));
	m_basicMaterial.metallic = 0.3f;
	m_basicMaterial.roughness = 0.75f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_basicMaterial, glm::vec3(-2.5f, 0.0f, 0.0f)));
	m_basicMaterial.metallic = 0.5f;
	m_basicMaterial.roughness = 0.5f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_basicMaterial, glm::vec3(0.0f, 0.0f, 0.0f)));
	m_basicMaterial.metallic = 0.75f;
	m_basicMaterial.roughness = 0.25f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_basicMaterial, glm::vec3(2.5f, 0.0f, 0.0f)));
	m_basicMaterial.metallic = 0.9f;
	m_basicMaterial.roughness = 0.0f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_basicMaterial, glm::vec3(5.0f, 0.0f, 0.0f)));

	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_texturedMaterial, glm::vec3(0.0f, 3.0f, 0.0f)));

	//scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SUZANNE], m_basicMaterial, glm::vec3(0.0f, 0.0f, 0.0f)));

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
	ImGui_ImplOpenGL3_Init("#version 450");

	// Setup mesh selecction dropdown data
	auto meshTypeNames = magic_enum::enum_names<MeshType>(); 
	for (const auto& name : meshTypeNames) {
		m_meshTypes.push_back(name.data());
	}
}

void Application::updateUI()
{
	Camera* cam = m_renderer->getCamera();
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

	ImGui::Begin("Scene Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// Entity list
	for (auto& entity : currentScene->getEntities()) {
		if (ImGui::TreeNode(entity->getName().c_str())) {
			if (ImGui::TreeNode("Material")) {
				Material& material = entity->getMaterial();
				if (!material.useAlbedoMap) {
					ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
				}
				else {
					ImGui::Text("Albedo Map");
				}
				if (!material.useMetalMap) {
					ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);
				}
				else {
					ImGui::Text("Metallic Map");
				}
				if (!material.useRoughMap) {
					ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
				}
				else {
					ImGui::Text("Roughness Map");
				}
				if (!material.useAoMap) {
					ImGui::SliderFloat("AO", &material.ao, 0.0f, 1.0f);
				}
				else {
					ImGui::Text("AO Map");
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Transform")) {
				ImGui::InputFloat3("Position", glm::value_ptr(entity->position));
				ImGui::InputFloat3("Rotation", glm::value_ptr(entity->rotation));
				ImGui::InputFloat3("Scale", glm::value_ptr(entity->scale));

				if (ImGui::Button("Reset Transform")) {
					entity->position = glm::vec3(0.0f);
					entity->rotation = glm::vec3(0.0f);
					entity->scale = glm::vec3(1.0f);
				}
				ImGui::TreePop();
			}
			
			if (cam->hasTarget() && cam->getTarget() == entity) {
				if (ImGui::Button("Unfollow")) {
					cam->setTarget(nullptr);
				}
			}
			else {
				if (ImGui::Button("Follow")) {
					cam->setTarget(entity);
				}
			}
			if (ImGui::Button("Delete Entity")) {
				currentScene->deleteEntity(entity);
			}
			ImGui::TreePop();
		}
	}


	bool& isAddingEntity = currentScene->getIsAddingEntity();
	glm::vec3& newPosition = currentScene->getNewEntityPosition();

	// Add entity button
	if (ImGui::Button("Add new Entity")) {
		isAddingEntity = true;  
	}

	ImGui::End();

	if (isAddingEntity) {
		ImGui::SetNextWindowPos(ImVec2(100, 600), ImGuiCond_Always);

		ImGui::Begin("Create Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::InputFloat3("Position", glm::value_ptr(newPosition));

		ImGui::Combo("Mesh Type", &m_meshTypeIndex, m_meshTypes.data(), static_cast<int>(m_meshTypes.size()));
		MeshType selectedMeshType = magic_enum::enum_value<MeshType>(m_meshTypeIndex);

		if (ImGui::Button("Confirm Entity")) {
			Material material;
			material.shader = m_renderer->getPBRShader();
			material.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
			material.metallic = 1.0f;
			material.roughness = 0.1f;
			material.ao = 0.1f;

			currentScene->addEntity(std::make_shared<Entity>(m_meshes[selectedMeshType], material, newPosition));
			isAddingEntity = false;  
			newPosition = glm::vec3(0.0f); 
		}

		if (ImGui::Button("Cancel")) {
			isAddingEntity = false;
			newPosition = glm::vec3(0.0f); 
		}
		ImGui::End();
	}

}

void Application::processInput(float deltaTime)
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

	Camera* cam = m_renderer->getCamera();
	if (glfwGetMouseButton(m_renderer->getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		
		cam->lookRotate(deltaTime, xoffset, yoffset);

		// Hide cursor
		glfwSetInputMode(m_renderer->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} 
	else if (glfwGetMouseButton(m_renderer->getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		// Show cursor
		glfwSetInputMode(m_renderer->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (!cam->hasTarget()) {
		// if wasd keys are pressed, move the camera
		if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_W) == GLFW_PRESS) {
			cam->moveForward(deltaTime);
		}
		if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_S) == GLFW_PRESS) {
			cam->moveBackward(deltaTime);
		}
		if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_A) == GLFW_PRESS) {
			cam->moveLeft(deltaTime);
		}
		if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_D) == GLFW_PRESS) {
			cam->moveRight(deltaTime);
		}
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

void Application::deltaTime()
{
	m_currentFrame = glfwGetTime();
	m_deltaTime = m_currentFrame - m_lastFrame;
	m_lastFrame = m_currentFrame;
}

