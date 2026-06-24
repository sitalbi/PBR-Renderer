#include "application.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>
#include <future>
#include <magic_enum.hpp>
#include <imgui_internal.h>
#include <filesystem>
#include "ModelLoader.h"
#include <ImGuizmo.h>

Application::Application()
{
	m_lastX = 0;
	m_lastY = 0;
	m_firstMouse = true;
}

Application::~Application()
{
}

void Application::run()
{
	while (!glfwWindowShouldClose(m_window))
	{
		deltaTime();

		update(m_deltaTime);

		if (m_showEditorUI)
		{
			updateUI();
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		glfwSwapBuffers(m_window);
		glfwPollEvents();

		processInput(m_deltaTime);
	}
	shutdown();
}

void Application::init()
{

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_SAMPLES, 8);
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

	m_currentScene.dirLight.lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	m_renderer.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
	m_renderer.init();

	m_camera = Camera(m_renderer.m_renderTargetResolution.width, m_renderer.m_renderTargetResolution.height, glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f);

	initUI();

	setCallbacks();

	m_currentScene.skyboxHandle = m_renderer.createSkyboxFromHDR(RES_DIR"/textures/skybox/kiara_1_dawn_4k.hdr");

	// Create default material
	MaterialData basicMat;
	basicMat.albedo = glm::vec3(0.5f, 0.5f, 0.5f);
	basicMat.metallic = 0.1f;
	basicMat.roughness = 0.9f;
	basicMat.emissiveColor = glm::vec3(0.0f, 0.0f, 0.0f);
	m_defaultMaterial = m_renderer.createMaterial(basicMat);

	// Create sphere
	MeshHandle sphereMesh = m_renderer.createMesh(MeshFactory::makeSphere(1.0f, 36.f));

	m_sphereModel = std::make_shared<Model>();
	m_sphereModel->submeshes.push_back({ sphereMesh, m_defaultMaterial });

	// Create cube
	MeshHandle cubeMesh = m_renderer.createMesh(MeshFactory::makeCube(1.0f));

	m_cubeModel = std::make_shared<Model>();
	m_cubeModel->submeshes.push_back({ cubeMesh, m_defaultMaterial });

	// Create Entities
	std::shared_ptr<Entity> sphere = std::make_shared<Entity>(m_sphereModel, glm::vec3(-2.0f, 0.0f, 0.0f), "Sphere");
	std::shared_ptr<Entity> sphere2 = std::make_shared<Entity>(m_sphereModel, glm::vec3(2.0f, 0.0f, 0.0f), "Sphere2");
	std::shared_ptr<Entity> cube = std::make_shared<Entity>(m_cubeModel, glm::vec3(0.0f, -2.0f, 0.0f), "Cube");
	cube->scale = glm::vec3(10.0f, 0.1f, 10.0f);
	// Add Entities to scene
	/*m_currentScene.addEntity(sphere);
	m_currentScene.addEntity(sphere2);
	m_currentScene.addEntity(cube);*/

	/*Model sponza = ModelLoader::loadModel(RES_DIR"/models/sponza/sponza.obj", m_renderer);
	std::shared_ptr<Entity> sponzaEntity = std::make_shared<Entity>(std::make_shared<Model>(sponza), glm::vec3(0.0f, 0.0f, 0.0f), "Sponza");
	sponzaEntity->scale = glm::vec3(0.02f);*/

	//m_currentScene.addEntity(sponzaEntity);

	Model kabuto = ModelLoader::loadModel(RES_DIR"/models/pbr_kabuto_samurai_helmet/kabuto.gltf", m_renderer);
	//Model grunt = ModelLoader::loadModel(RES_DIR"/models/UnitTroopWFGrunt/UnitTroopWFGrunt.bgf.obj", m_renderer);
	std::shared_ptr<Entity> kabutoEntity = std::make_shared<Entity>(std::make_shared<Model>(kabuto), glm::vec3(0.0f, 0.0f, 0.0f), "Kabuto");
	//std::shared_ptr<Entity> gruntEntity = std::make_shared<Entity>(std::make_shared<Model>(grunt), glm::vec3(0.0f, 0.0f, 0.0f), "Grunt");

	m_currentScene.addEntity(kabutoEntity);

	PointLightData data;
	data.color = glm::vec3(1.f);
	data.position = glm::vec3(0.f, 5.f, 0.f);
	data.intensity = 100.f;
	
	//PointLightHandle pointLight = m_renderer.createPointLight(data);

	//m_currentScene.addPointLight(pointLight);
}

void Application::shutdown()
{
	m_renderer.shutdown();
}

void Application::initUI()
{
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	setupImGuiStyle();
}

void Application::updateUI()
{
	// Start a new frame for ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	// Info Panel 
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200, 500), ImGuiCond_FirstUseEver);
	ImGui::SetWindowCollapsed(false, ImGuiCond_FirstUseEver);
	ImGui::Begin("Info");
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(10, 120), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	ImGui::SetWindowCollapsed(false, ImGuiCond_FirstUseEver);
	ImGui::Begin("Post-Processing");
	ImGui::Checkbox("SSAO", &m_renderer.useSSAO);
	ImGui::Checkbox("Bloom", &m_renderer.useBloom);
	ImGui::Checkbox("Anti-Aliasing", &m_renderer.useAA);
	ImGui::End();


	ImGui::SetNextWindowPos(ImVec2(350, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
	ImGui::SetWindowCollapsed(false, ImGuiCond_FirstUseEver);
	ImGui::Begin("Environment Settings");
	ImGui::SetNextItemWidth(100.0f);
	ImGui::SliderFloat("Exposure", &m_currentScene.dirLight.exposure, 0.01f, 1.0f);

	static float lightYaw = 90.0f;
	static float lightPitch = -45.0f;

	ImGui::Text("Directional Light");
	ImGui::SetNextItemWidth(150.0f);
	bool changed = ImGui::SliderFloat("Yaw", &lightYaw, -180.0f, 180.0f);
	ImGui::SetNextItemWidth(150.0f);
	changed |= ImGui::SliderFloat("Pitch", &lightPitch, -89.0f, 89.0f);

	if (changed)
	{
		float yawRad = glm::radians(lightYaw);
		float pitchRad = glm::radians(lightPitch);

		glm::vec3 dir;
		dir.x = std::cos(pitchRad) * std::cos(yawRad);
		dir.y = std::sin(pitchRad);
		dir.z = std::cos(pitchRad) * std::sin(yawRad);

		m_currentScene.dirLight.lightDir = glm::normalize(dir);
	}
	ImGui::Text("Light Intensity");
	ImGui::SliderFloat("##light", &m_currentScene.dirLight.lightIntensity, 0.0f, 10.0f);
	ImGui::Text("Ambient Intensity");
	ImGui::SliderFloat("##ambient", &m_currentScene.dirLight.ambientIntensity, 0.0f, 1.0f);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(350, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
	ImGui::SetWindowCollapsed(false, ImGuiCond_FirstUseEver);
	ImGui::Begin("Scene Editor");

	// Entity list
	for (auto& entity : m_currentScene.getEntities()) {
		if (ImGui::TreeNode(entity->getName().c_str())) {

			if (ImGui::TreeNode("Materials")) {
				const auto& submeshes = entity->getModel()->getSubmeshes();

				for (int i = 0; i < static_cast<int>(submeshes.size()); ++i) {
					if (ImGui::TreeNode(("Material##" + std::to_string(i)).c_str())) {

						MaterialHandle material = entity->hasMaterialOverride(i) ? entity->getMaterialOverride(i) : submeshes[i].materialHandle;

						MaterialData data = m_renderer.getMaterialData(material);

						bool changed = false;

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);

						changed |= ImGui::ColorEdit3("Albedo", glm::value_ptr(data.albedo));
						changed |= ImGui::SliderFloat("Metallic", &data.metallic, 0.0f, 1.0f);
						changed |= ImGui::SliderFloat("Roughness", &data.roughness, 0.0f, 1.0f);
						changed |= ImGui::ColorEdit3("Emissive Color", glm::value_ptr(data.emissiveColor), ImGuiColorEditFlags_HDR);

						ImGui::PopItemWidth();

						if (changed) {
							if (!entity->hasMaterialOverride(i)) {
								MaterialHandle newMat = m_renderer.createMaterial(data);
								entity->setMaterial(i, newMat);
							}
							else {
								m_renderer.editMaterial(material, data);
							}
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Transform")) {
				ImGui::DragFloat3("Position", glm::value_ptr(entity->position), 1.0f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(entity->rotation), 0.1f);
				ImGui::DragFloat3("Scale", glm::value_ptr(entity->scale), 0.1f, 0.1f, 200.0f);

				if (ImGui::Button("Reset Transform")) {
					entity->position = glm::vec3(0.0f);
					entity->rotation = glm::vec3(0.0f);
					entity->scale = glm::vec3(1.0f);
				}
				ImGui::TreePop();
			}
			
			
			if (m_camera.hasTarget() && m_camera.getTarget() == entity) {
				if (ImGui::Button("Unfollow")) {
					m_camera.setTarget(nullptr);
				}
			}
			else {
				if (ImGui::Button("Follow")) {
					m_camera.setTarget(entity);
				}
			}
			if (selectedEntity != entity.get())
			{
				if (ImGui::Button("Select"))
				{
					selectedEntity = entity.get();
				}
			}
			else
			{
				if (ImGui::Button("Unselect"))
				{
					selectedEntity = nullptr;
				}
			}

			if (ImGui::Button("Delete Entity")) {
				m_currentScene.deleteEntity(entity);
			}
			ImGui::TreePop();
		}
	}

	// PointLights List
	for (auto& pointLight : m_currentScene.getPointLights()) {
		if (ImGui::TreeNode("Point Light")) {

			PointLightData data = m_renderer.getPointLightData(pointLight);

			bool changed = false;

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);

			changed |= ImGui::DragFloat3("Position", glm::value_ptr(data.position), 1.0f);
			changed |= ImGui::ColorEdit3("Color", glm::value_ptr(data.color));
			changed |= ImGui::SliderFloat("Intensity", &data.intensity, 0.0f, 300.0f);
			changed |= ImGui::SliderFloat("Constant", &data.constant, 0.0f, 1.0f);
			changed |= ImGui::SliderFloat("Linear", &data.linear, 0.0f, 0.5f);
			changed |= ImGui::SliderFloat("Quadratic", &data.quadratic, 0.0f, 0.1f);

			ImGui::PopItemWidth();

			if (changed) {
				m_renderer.editPointLight(pointLight, data);
			}

			ImGui::TreePop();
		
		}
	}

	bool& isAddingEntity = m_currentScene.getIsAddingEntity();
	glm::vec3& newPosition = m_currentScene.getNewEntityPosition();

	// Add entity button
	if (ImGui::Button("Add new Entity")) {
		isAddingEntity = true;  
	}


	if (isAddingEntity) {

		ImGui::Begin("Create Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::InputFloat3("Position", glm::value_ptr(newPosition));

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
				for (const auto& entity : m_currentScene.getEntities()) {
					if (entity->getName() == entityName) {
						// Generate a new name with a numeric suffix
						snprintf(entityName, sizeof(entityName), "%s%d", baseName.c_str(), suffix);
						++suffix;
						nameExists = true;
						break;
					}
				}
			} while (nameExists);

			m_currentScene.addEntity(std::make_shared<Entity>(m_sphereModel, newPosition, entityName));
			isAddingEntity = false;
			newPosition = glm::vec3(0.0f);

			// reset material index and entity name
			//materialIndex = 0;
			memset(entityName, 0, sizeof(entityName));
		}

		if (ImGui::Button("Cancel")) {
			isAddingEntity = false;
			newPosition = glm::vec3(0.0f); 
		}
		ImGui::End();
	}

	ImGui::End();

	updateGizmo();

	ImGui::Render();
}

void Application::update(float deltaTime)
{
	RenderData renderData = buildRenderData();
	m_renderer.render(renderData);
}

void Application::processInput(float deltaTime)
{
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(m_window, true); 
	}

	onPressedKey(GLFW_KEY_F2, [&]() {
		m_showEditorUI = !m_showEditorUI;
	});

	
	// TODO: refactor using callbacks
	double xpos, ypos;
	glfwGetCursorPos(m_window, &xpos, &ypos);
	static double lastX = xpos;
	static double lastY = ypos;
	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		
		m_camera.lookRotate(deltaTime, xoffset, yoffset);

		// Hide cursor
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} 
	else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		// Show cursor
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (!m_camera.hasTarget()) {
		// if wasd keys are pressed, move the camera
		if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
			m_camera.moveForward(deltaTime);
		}
		if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
			m_camera.moveBackward(deltaTime);
		}
		if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
			m_camera.moveLeft(deltaTime);
		}
		if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
			m_camera.moveRight(deltaTime);
		}
		if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			m_camera.moveUp(deltaTime);
		}
		if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			m_camera.moveDown(deltaTime);
		}
	}
}

void Application::setCallbacks()
{
	glfwSetWindowUserPointer(m_window, &m_camera);

	glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
		Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
		cam->scroll(yoffset);
	});
}

RenderData Application::buildRenderData()
{
	RenderData data{};

	// Camera
	data.camera.position = m_camera.getPosition();
	data.camera.viewMatrix = m_camera.getViewMatrix();
	data.camera.projectionMatrix = m_camera.getProjectionMatrix();

	// Skybox
	data.skybox = m_currentScene.skyboxHandle;

	// Meshes
	for (const auto& entity : m_currentScene.getEntities())
	{
		if (!entity->getModel())
			continue;

		glm::mat4 modelMatrix = entity->getModelMatrix();

		const auto& submeshes = entity->getModel()->getSubmeshes();
		for (size_t i = 0; i < submeshes.size(); ++i)
		{
			RenderMesh rm{};
			rm.mesh = submeshes[i].meshHandle;
			rm.material = entity->hasMaterialOverride(i)
				? entity->getMaterialOverride(i)
				: submeshes[i].materialHandle;
			rm.transform = modelMatrix;

			data.meshes.push_back(rm);
		}
	}

	// Point Lights
	for (const auto& light : m_currentScene.getPointLights())
	{
		data.lights.push_back(light);
	}

	// Directional Light
	data.directionalLight = m_currentScene.dirLight;

	return data;
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
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
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

void Application::onPressedKey(int key, const std::function<void()>& callback)
{
	bool isPressed = glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS;
	if (isPressed && !m_keyStates[key]) {
		callback();
	}
	m_keyStates[key] = isPressed;

}

void Application::updateGizmo()
{
	if (!selectedEntity)
	{
		return;
	}

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	ImGuiIO& io = ImGui::GetIO();

	ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);

	glm::mat4 view = m_camera.getViewMatrix();
	glm::mat4 projection = m_camera.getProjectionMatrix();
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), selectedEntity->position);
	transform *= glm::mat4_cast(glm::quat(glm::radians(selectedEntity->rotation)));
	transform = glm::scale(transform, selectedEntity->scale);

	ImGuizmo::Manipulate(
		glm::value_ptr(view),
		glm::value_ptr(projection),
		ImGuizmo::TRANSLATE | ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z | ImGuizmo::SCALE,
		ImGuizmo::WORLD,
		glm::value_ptr(transform)
	);

	if (ImGuizmo::IsUsing())
	{
		float translation[3];
		float rotation[3];
		float scale[3];

		ImGuizmo::DecomposeMatrixToComponents(
			glm::value_ptr(transform),
			translation,
			rotation,
			scale
		);

		selectedEntity->position = glm::vec3(translation[0], translation[1], translation[2]);
		selectedEntity->rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);
		selectedEntity->scale = glm::vec3(glm::max(0.001f, (scale[0] + scale[1] + scale[2]) / 3.0f));
	}
}