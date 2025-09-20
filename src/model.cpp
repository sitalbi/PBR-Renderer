#include "model.h"

std::shared_ptr<Material> Model::defaultMaterial = std::make_shared<Material>();

Model::Model()
{
}

Model::~Model()
{
}

void Model::loadModel(const std::string& path)
{
}

void Model::loadSphere(float radius, unsigned int segments)
{
	submeshes.clear();
	Mesh sphereMesh;
	sphereMesh.loadSphere(radius, segments);
	sphereMesh.material = defaultMaterial;
	submeshes.push_back(sphereMesh);
}

void Model::loadCube(float size)
{
	submeshes.clear();
	Mesh cubeMesh;
	cubeMesh.loadCube(size);
	cubeMesh.material = defaultMaterial;
	submeshes.push_back(cubeMesh);
}

void Model::draw()
{
	for (auto& mesh : submeshes) {
		mesh.material->bind();
		mesh.draw();
	}
}