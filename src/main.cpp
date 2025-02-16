#include "renderer.h"
#include "camera.h"


int main() {
	Renderer renderer;
	renderer.init();

	auto basicShader = std::make_shared<Shader>(RES_DIR "/shaders/basic_vert.glsl", RES_DIR  "/shaders/basic_frag.glsl");
	
	auto mesh = std::make_shared<Mesh>();

	renderer.addEntity(std::make_unique<Entity>(basicShader, mesh));

	while (!glfwWindowShouldClose(renderer.getWindow()))
	{
		renderer.update();
	}
	renderer.shutdown();

	return 0;
}