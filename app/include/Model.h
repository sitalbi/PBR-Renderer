#pragma once
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <renderer/RenderData.h>

enum class ModelType
{
	Sphere,
	Cube,
	Suzanne,
	Kabuto
};


struct Submesh
{
	MeshHandle meshHandle;
	MaterialHandle materialHandle;
};

class Model
{
public:
	std::vector<Submesh> submeshes;

	std::vector<Submesh>& getSubmeshes() { return submeshes; }
};