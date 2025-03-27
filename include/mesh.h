#pragma once

#include "vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

enum class MeshType
{
	CUBE,
	SPHERE,
	SUZANNE,
	KABUTO
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
	void loadCube();
	void loadSphere(float radius, unsigned int segments);

private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;

	unsigned int m_vao, m_vbo, m_ibo;

	bool isSetup = false;

};