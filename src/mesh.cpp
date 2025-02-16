#include "mesh.h"
#include "glad/glad.h"



Mesh::Mesh()
{
	loadCube();
}

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds) : m_vertices(verts), m_indices(inds)
{
	setupMesh();
}

Mesh::~Mesh()
{
}

void Mesh::setupMesh()
{
	// Generate buffers
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
	glEnableVertexAttribArray(1);

	// Texture coordinates attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_texCoords));
	glEnableVertexAttribArray(2);

	// Tangent attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_tangent));
	glEnableVertexAttribArray(3);

	// Bitangent attribute
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_bitangent));
	glEnableVertexAttribArray(4);

	// Unbind the VAO
	glBindVertexArray(0);
}

void Mesh::loadModel(const char* path)
{
	// TODO: Implement loading models from file
}

void Mesh::draw()
{
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::loadCube()
{
	m_vertices = cubeVertices;
	m_indices = cubeIndices;

	setupMesh();
}