#include "mesh.h"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"

Mesh::Mesh()
{
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
	// Cube data
	const std::vector<Vertex> cubeVertices = {
		// Front face
		Vertex(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
		// Back face
		Vertex(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
		// Top face
		Vertex(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		// Bottom face
		Vertex(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		// Right face
		Vertex(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		// Left face
		Vertex(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f))
	};

	const std::vector<unsigned int> cubeIndices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};
	m_vertices = cubeVertices;
	m_indices = cubeIndices;

	setupMesh();
}

void Mesh::loadSphere(float radius, unsigned int segments)
{
	const float pi = glm::pi<float>();
	const float pi2 = 2.0f * pi;

	for (unsigned int y = 0; y <= segments; ++y) {
		for (unsigned int x = 0; x <= segments; ++x) {
			float xSegment = (float)x / (float)segments;
			float ySegment = (float)y / (float)segments;

			glm::vec3 pos;
			pos.x = std::cos(xSegment * pi2) * std::sin(ySegment * pi) * radius;
			pos.y = std::cos(ySegment * pi) * radius;
			pos.z = std::sin(xSegment * pi2) * std::sin(ySegment * pi) * radius;

			glm::vec3 norm = glm::normalize(pos);
			glm::vec2 texCoord(xSegment, ySegment);

			Vertex vertex;
			vertex.m_position = pos;
			vertex.m_normal = norm;
			vertex.m_texCoords = texCoord;
			m_vertices.push_back(vertex);
		}
	}

	for (unsigned int y = 0; y <= segments; ++y) {
		for (unsigned int x = 0; x <= segments; ++x) {
			m_indices.push_back((y + 1) * (segments + 1) + x);
			m_indices.push_back(y * (segments + 1) + x);
			m_indices.push_back(y * (segments + 1) + x + 1);
			
			m_indices.push_back((y + 1) * (segments + 1) + x);
			m_indices.push_back(y * (segments + 1) + x + 1);
			m_indices.push_back((y + 1) * (segments + 1) + x + 1);
		}
	}
	setupMesh();
}
