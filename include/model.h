#pragma once
#include "mesh.h"
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

	void processNode(aiNode* node, const aiScene* scene, const std::string& directory);
	Mesh processMesh(aiMesh* mesh);
	std::shared_ptr<Material> loadMaterial(aiMaterial* aiMat, const std::string& directory);
};