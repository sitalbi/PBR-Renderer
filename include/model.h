#pragma once
#include "mesh.h"
#include <string>

class Model
{
public:
	Model();
	~Model();

	void loadModel(const std::string& path);
	void loadSphere(float radius, unsigned int segments);
	void loadCube(float size);
	void draw();

	std::vector<Mesh>& getSubmeshes() { return submeshes; }

	static std::shared_ptr<Material> defaultMaterial;

private:

	std::vector<Mesh> submeshes;

};