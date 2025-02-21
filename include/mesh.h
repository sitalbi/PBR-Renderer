#pragma once

#include "vertex.h"




enum class MeshType
{
	CUBE,
	SPHERE
};

class Mesh
{
public:
	Mesh();
	Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds);
	~Mesh();

	void setupMesh();
	void loadModel(const char* path);
	void draw();
	void loadCube();
	void loadSphere(float radius, unsigned int segments);

private:
	float M_PI = 3.14159265359f;

	unsigned int m_vao, m_vbo, m_ibo;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;

};