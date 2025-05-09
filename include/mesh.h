#pragma once

#include "vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

enum class MeshType
{
	Sphere,
	Cube,
	Suzanne,
	Kabuto
};

class Mesh
{
public:
	Mesh();
	Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds);
	~Mesh();

	void setupMesh();
	void loadModel(const std::string& path);
	void draw();
	void loadSphere(float radius, unsigned int segments);
	void loadCube(float size);

private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;

	unsigned int m_vao, m_vbo, m_ibo;

	bool isSetup = false;

};