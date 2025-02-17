#include "application.h"

Application::Application()
{
	m_renderer = std::make_unique<Renderer>();
	m_camera = Camera(window_width, window_height, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
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
		m_renderer->update();
		glfwPollEvents();

		processInput();
	}
	shutdown();
}

void Application::init()
{
	m_renderer->init();
	m_renderer->setCamera(&m_camera);

	setCallbacks();

	m_basicShader = std::make_shared<Shader>(RES_DIR "/shaders/basic_vert.glsl", RES_DIR  "/shaders/basic_frag.glsl");

	m_basicMesh = std::make_shared<Mesh>();

	m_renderer->addEntity(std::make_unique<Entity>(m_basicShader, m_basicMesh));
}

void Application::shutdown()
{
	m_renderer->shutdown();
}

void Application::processInput()
{
	if (glfwGetKey(m_renderer->getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(m_renderer->getWindow(), true); 
	}
}

void Application::setCallbacks()
{
	GLFWwindow* window = m_renderer->getWindow();

	glfwSetWindowUserPointer(window, &m_camera);
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else if (action == GLFW_RELEASE) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
		});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		static double lastX = xpos;
		static double lastY = ypos;
		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
			cam->orbit(xoffset, yoffset);
		}
		});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
		cam->zoom(yoffset);
		});
}

