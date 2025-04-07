#include "mesh.h"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

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

	isSetup = true;
}



void Mesh::draw()
{
	if (!isSetup)
	{
		setupMesh();
	}
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::loadSphere(float radius, unsigned int segments)
{
	const float pi = glm::pi<float>();
	const float pi2 = 2.0f * pi;

	for (unsigned int y = 0; y < segments; ++y) {
		for (unsigned int x = 0; x < segments; ++x) {
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


void Mesh::loadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	processNode(scene->mRootNode, scene);
	setupMesh();
}


void Mesh::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene);
	}
}

void Mesh::processMesh(aiMesh* mesh, const aiScene* scene)
{
	m_vertices.clear();
	m_indices.clear();

	// Vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		// Positions
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.m_position = vector;

		// Normals
		if (mesh->HasNormals()) {
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.m_normal = vector;
		}

		// Texture coordinates
		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.m_texCoords = vec;
		}
		else {
			vertex.m_texCoords = glm::vec2(0.0f, 0.0f);
		}

		// Tangents
		if (mesh->HasTangentsAndBitangents()) {
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.m_tangent = vector;

			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.m_bitangent = vector;
		}

		m_vertices.push_back(vertex);
	}

	// Indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			m_indices.push_back(face.mIndices[j]);
		}
	}
}