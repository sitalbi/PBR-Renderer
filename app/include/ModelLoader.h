#pragma once

#include "Model.h"
#include <renderer/Renderer.h>

namespace ModelLoader
{
	Model loadModel(const std::string& path, Renderer& renderer);
	void processNode(aiNode* node, const aiScene* scene, Renderer& renderer, const std::string& directory, Model& model);
	MeshHandle processMesh(aiMesh* mesh, Renderer& renderer);
	MaterialHandle loadMaterial(aiMaterial* aiMat, const std::string& directory, Renderer& renderer);
}

namespace MeshFactory
{
	MeshData makeSphere(float radius, unsigned int segments);
	MeshData makeCube(float size);
}