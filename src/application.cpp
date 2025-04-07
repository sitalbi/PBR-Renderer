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
	std::shared_ptr<Mesh> m_sphereMesh = std::make_shared<Mesh>();
	m_sphereMesh->loadSphere(1.0f, 50);
	m_meshes[MeshType::Sphere] = m_sphereMesh;


	std::shared_ptr<Mesh> m_suzanneMesh = std::make_shared<Mesh>();
	m_suzanneMesh->loadModel(RES_DIR"/models/suzanne.obj");
	m_meshes[MeshType::Suzanne] = m_suzanneMesh;

	std::shared_ptr<Mesh> m_kabutoMesh = std::make_shared<Mesh>();
	m_kabutoMesh->loadModel(RES_DIR"/models/kabuto.obj");
	m_meshes[MeshType::Kabuto] = m_kabutoMesh;

	// Create default material
	std::shared_ptr<Material> basicMat = std::make_shared<Material>();
	basicMat->shader = m_renderer->getPBRShader();
	basicMat->albedo = glm::vec3(1.0f, 0.0f, 0.0f);
	basicMat->metallic = 0.01f;
	basicMat->roughness = 0.9f;
	basicMat->ao = 0.25f;

	// Create textured materials
	std::shared_ptr<Material> lightgoldMat = std::make_shared<Material>();
	lightgoldMat->shader = m_renderer->getPBRShader();
	lightgoldMat->albedoMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_albedo.png");
	lightgoldMat->normalMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_normal-ogl.png");
	lightgoldMat->metallicMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_metallic.png");
	lightgoldMat->roughnessMap = std::make_shared<Texture>(RES_DIR"/textures/materials/lightgold_roughness.png");
	//lightgoldMat->aoMap = std::make_shared<Texture>(RES_DIR"/textures/materials/scuffed-plastic-ao.png");
	lightgoldMat->ao = 0.5f;

	lightgoldMat->useAlbedoMap = true;
	lightgoldMat->useNormalMap = true;
	lightgoldMat->useMetalMap = true;
	lightgoldMat->useRoughMap = true;
	//lightgoldMat->useAoMap = true;

	std::shared_ptr<Material> scuffedPlasticMat = std::make_shared<Material>();
	scuffedPlasticMat->shader = m_renderer->getPBRShader();
	scuffedPlasticMat->albedoMap = std::make_shared<Texture>(RES_DIR"/textures/materials/scuffed-plastic-alb.png");
	scuffedPlasticMat->normalMap = std::make_shared<Texture>(RES_DIR"/textures/materials/scuffed-plastic-normal.png");
	scuffedPlasticMat->metallicMap = std::make_shared<Texture>(RES_DIR"/textures/materials/scuffed-plastic-metal.png");
	scuffedPlasticMat->roughnessMap = std::make_shared<Texture>(RES_DIR"/textures/materials/scuffed-plastic-rough.png");
	scuffedPlasticMat->aoMap = std::make_shared<Texture>(RES_DIR"/textures/materials/scuffed-plastic-ao.png");

	scuffedPlasticMat->useAlbedoMap = true;
	scuffedPlasticMat->useNormalMap = true;
	scuffedPlasticMat->useMetalMap = true;
	scuffedPlasticMat->useRoughMap = true;
	scuffedPlasticMat->useAoMap = true;

	// Set materials to map
	m_materials["Default"] = basicMat;
	m_materials["Light Gold"] = lightgoldMat;
	m_materials["Scuffed Plastic"] = scuffedPlasticMat;

	// Kabuto material
	Material m_kabutoMaterial;
	m_kabutoMaterial.shader = m_renderer->getPBRShader();
	m_kabutoMaterial.albedoMap = std::make_shared<Texture>(RES_DIR"/textures/materials/kabuto/Material_baseColor.png");
	m_kabutoMaterial.normalMap = std::make_shared<Texture>(RES_DIR"/textures/materials/kabuto/Material_normal.png");
	m_kabutoMaterial.metallicMap = std::make_shared<Texture>(RES_DIR"/textures/materials/kabuto/Material_metallic.png");
	m_kabutoMaterial.roughnessMap = std::make_shared<Texture>(RES_DIR"/textures/materials/kabuto/Material_roughness.png");
	m_kabutoMaterial.ao = 0.5f;

	m_kabutoMaterial.useAlbedoMap = true;
	m_kabutoMaterial.useNormalMap = true;
	m_kabutoMaterial.useMetalMap = true;
	m_kabutoMaterial.useRoughMap = true;

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	/*scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], basicMat, glm::vec3(-5.0f, 0.0f, 0.0f)));
	basicMat.metallic = 0.3f;
	basicMat.roughness = 0.75f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], basicMat, glm::vec3(-2.5f, 0.0f, 0.0f)));
	basicMat.metallic = 0.5f;
	basicMat.roughness = 0.5f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], basicMat, glm::vec3(0.0f, 0.0f, 0.0f)));
	basicMat.metallic = 0.75f;
	basicMat.roughness = 0.25f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], basicMat, glm::vec3(2.5f, 0.0f, 0.0f)));
	basicMat.metallic = 0.9f;
	basicMat.roughness = 0.0f;
	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], basicMat, glm::vec3(5.0f, 0.0f, 0.0f)));

	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::SPHERE], m_lightgoldMat, glm::vec3(0.0f, 3.0f, 0.0f)));*/

	scene->addEntity(std::make_shared<Entity>(m_meshes[MeshType::Kabuto], m_kabutoMaterial, glm::vec3(0.0f, 0.0f, 0.0f), "Kabuto"));

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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_renderer->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 450");

	setupImGuiStyle();

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

	// Start a new frame for ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Use main window as dock space
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

	// Info Panel 
	ImGui::Begin("Info");
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Checkbox("SSAO", &m_renderer->useSSAO);
	ImGui::End();

	ImGui::Begin("Scene Editor");

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

		ImGui::Begin("Create Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::InputFloat3("Position", glm::value_ptr(newPosition));

		ImGui::Combo("Mesh Type", &m_meshTypeIndex, m_meshTypes.data(), static_cast<int>(m_meshTypes.size()));
		MeshType selectedMeshType = magic_enum::enum_value<MeshType>(m_meshTypeIndex);

		// Combo box for materials using the unordered_map
		std::vector<const char*> materialNames;
		for (const auto& [key, value] : m_materials) {
			materialNames.push_back(key.c_str());
		}
		static int materialIndex = 0;
		ImGui::Combo("Material", &materialIndex, materialNames.data(), static_cast<int>(materialNames.size()));
		Material material = *m_materials[materialNames[materialIndex]];

		// Text input for entity name
		static char entityName[32];
		ImGui::InputText("Name", entityName, IM_ARRAYSIZE(entityName));

		if (ImGui::Button("Confirm Entity")) {
			// Use mesh type as default name if none is provided
			if (strlen(entityName) == 0) {
				strcpy(entityName, m_meshTypes[m_meshTypeIndex]);
			}

			// Ensure the entity name is unique
			std::string baseName(entityName);
			int suffix = 1;
			bool nameExists;
			do {
				nameExists = false;
				for (const auto& entity : currentScene->getEntities()) {
					if (entity->getName() == entityName) {
						// Generate a new name with a numeric suffix
						snprintf(entityName, sizeof(entityName), "%s%d", baseName.c_str(), suffix);
						++suffix;
						nameExists = true;
						break;
					}
				}
			} while (nameExists);

			currentScene->addEntity(std::make_shared<Entity>(m_meshes[selectedMeshType], material, newPosition, entityName));
			isAddingEntity = false;
			newPosition = glm::vec3(0.0f);

			// reset material index and entity name
			materialIndex = 0;
			memset(entityName, 0, sizeof(entityName));
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
		if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS) {
			cam->moveUp(deltaTime);
		}
		if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			cam->moveDown(deltaTime);
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

void Application::setupImGuiStyle()
{
	ImGui::StyleColorsDark();  // Set the base style first

	// Get the current style reference
	ImGuiStyle& style = ImGui::GetStyle();

	// General layout adjustments for a clean look
	style.WindowPadding = ImVec2(10, 10);
	style.WindowRounding = 5;
	style.FramePadding = ImVec2(5, 5);
	style.FrameRounding = 4;
	style.ItemSpacing = ImVec2(8, 8);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.IndentSpacing = 20;
	style.ScrollbarSize = 12;
	style.ScrollbarRounding = 5;
	style.GrabMinSize = 10;
	style.GrabRounding = 3;

	// Set up a custom color palette
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.26f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.06f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.25f, 0.26f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.26f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.50f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.50f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_NavWindowingHighlight] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_NavWindowingDimBg] = colors[ImGuiCol_Header];
	colors[ImGuiCol_ModalWindowDimBg] = colors[ImGuiCol_WindowBg];

}