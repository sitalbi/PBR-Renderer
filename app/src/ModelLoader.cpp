#include "ModelLoader.h"
#include <filesystem>
#include <renderer/Vertex.h>
#include <glm/ext/scalar_constants.hpp>
#include <renderer/Renderer.h>


MeshData MeshFactory::makeSphere(float radius, unsigned int segments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float pi = glm::pi<float>();
    const float pi2 = 2.0f * pi;

    // vertices
    for (unsigned int y = 0; y <= segments; ++y) {
        for (unsigned int x = 0; x <= segments; ++x) {
            float xSegment = static_cast<float>(x) / static_cast<float>(segments);
            float ySegment = static_cast<float>(y) / static_cast<float>(segments);

            glm::vec3 pos;
            pos.x = std::cos(xSegment * pi2) * std::sin(ySegment * pi) * radius;
            pos.y = std::cos(ySegment * pi) * radius;
            pos.z = std::sin(xSegment * pi2) * std::sin(ySegment * pi) * radius;

            Vertex vertex;
            vertex.m_position = pos;
            vertex.m_normal = glm::normalize(pos);
            vertex.m_texCoords = glm::vec2(xSegment, ySegment);

            vertices.push_back(vertex);
        }
    }

    // indices
    for (unsigned int y = 0; y < segments; ++y) {
        for (unsigned int x = 0; x < segments; ++x) {
            unsigned int i0 = y * (segments + 1) + x;
            unsigned int i1 = (y + 1) * (segments + 1) + x;
            unsigned int i2 = y * (segments + 1) + (x + 1);
            unsigned int i3 = (y + 1) * (segments + 1) + (x + 1);

            indices.push_back(i1);
            indices.push_back(i0);
            indices.push_back(i2);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    return { vertices, indices };
}

MeshData MeshFactory::makeCube(float size)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	float half = size * 0.5f;

	// Front face (z = +half)
	{
		glm::vec3 normal(0.0f, 0.0f, 1.0f);
		glm::vec3 tangent(1.0f, 0.0f, 0.0f);
		glm::vec3 bitangent(0.0f, -1.0f, 0.0f);
		// UVs go from (0,0) to (1,1)
		vertices.push_back(Vertex(glm::vec3(-half, -half, half), normal, glm::vec2(0.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, -half, half), normal, glm::vec2(1.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, half, half), normal, glm::vec2(1.0f, 1.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, half, half), normal, glm::vec2(0.0f, 1.0f), tangent, bitangent));

		// Indices: (0,1,2) and (2,3,0) for this face
		unsigned int baseIndex = 0;
		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 1);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 3);
		indices.push_back(baseIndex + 0);
	}

	// Back face (z = -half)
	{
		glm::vec3 normal(0.0f, 0.0f, -1.0f);

		glm::vec3 tangent(-1.0f, 0.0f, 0.0f);
		glm::vec3 bitangent(0.0f, -1.0f, 0.0f);
		unsigned int baseIndex = vertices.size();
		vertices.push_back(Vertex(glm::vec3(half, -half, -half), normal, glm::vec2(0.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, -half, -half), normal, glm::vec2(1.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, half, -half), normal, glm::vec2(1.0f, 1.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, half, -half), normal, glm::vec2(0.0f, 1.0f), tangent, bitangent));

		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 1);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 3);
		indices.push_back(baseIndex + 0);
	}

	// Left face (x = -half)
	{
		glm::vec3 normal(-1.0f, 0.0f, 0.0f);

		glm::vec3 tangent(0.0f, 0.0f, 1.0f);
		glm::vec3 bitangent(0.0f, -1.0f, 0.0f);
		unsigned int baseIndex = vertices.size();
		vertices.push_back(Vertex(glm::vec3(-half, -half, -half), normal, glm::vec2(0.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, -half, half), normal, glm::vec2(1.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, half, half), normal, glm::vec2(1.0f, 1.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, half, -half), normal, glm::vec2(0.0f, 1.0f), tangent, bitangent));

		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 1);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 3);
		indices.push_back(baseIndex + 0);
	}

	// Right face (x = +half)
	{
		glm::vec3 normal(1.0f, 0.0f, 0.0f);
		glm::vec3 tangent(0.0f, 0.0f, -1.0f);
		glm::vec3 bitangent(0.0f, -1.0f, 0.0f);
		unsigned int baseIndex = vertices.size();
		vertices.push_back(Vertex(glm::vec3(half, -half, half), normal, glm::vec2(0.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, -half, -half), normal, glm::vec2(1.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, half, -half), normal, glm::vec2(1.0f, 1.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, half, half), normal, glm::vec2(0.0f, 1.0f), tangent, bitangent));

		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 1);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 3);
		indices.push_back(baseIndex + 0);
	}

	// Top face (y = +half)
	{
		glm::vec3 normal(0.0f, 1.0f, 0.0f);

		glm::vec3 tangent(1.0f, 0.0f, 0.0f);
		glm::vec3 bitangent(0.0f, 0.0f, -1.0f);
		unsigned int baseIndex = vertices.size();
		vertices.push_back(Vertex(glm::vec3(-half, half, half), normal, glm::vec2(0.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, half, half), normal, glm::vec2(1.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, half, -half), normal, glm::vec2(1.0f, 1.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, half, -half), normal, glm::vec2(0.0f, 1.0f), tangent, bitangent));

		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 1);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 3);
		indices.push_back(baseIndex + 0);
	}

	// Bottom face (y = -half)
	{
		glm::vec3 normal(0.0f, -1.0f, 0.0f);

		glm::vec3 tangent(1.0f, 0.0f, 0.0f);
		glm::vec3 bitangent(0.0f, 0.0f, 1.0f);
		unsigned int baseIndex = vertices.size();
		vertices.push_back(Vertex(glm::vec3(-half, -half, -half), normal, glm::vec2(0.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, -half, -half), normal, glm::vec2(1.0f, 0.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(half, -half, half), normal, glm::vec2(1.0f, 1.0f), tangent, bitangent));
		vertices.push_back(Vertex(glm::vec3(-half, -half, half), normal, glm::vec2(0.0f, 1.0f), tangent, bitangent));

		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 1);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 3);
		indices.push_back(baseIndex + 0);
	}

	return { vertices, indices };
}

Model ModelLoader::loadModel(const std::string& path, Renderer& renderer)
{
    Model model;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace
    );

    std::filesystem::path filepath(path);
    std::string directory = filepath.parent_path().string();

    processNode(scene->mRootNode, scene, renderer, directory, model);

    return model;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, Renderer& renderer, const std::string& directory, Model& model)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];

        MeshHandle meshHandle = processMesh(mesh, renderer);
        MaterialHandle matHandle = loadMaterial(aiMat, directory, renderer);

        model.submeshes.push_back({ meshHandle, matHandle });
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    { 
        processNode(node->mChildren[i], scene, renderer, directory, model);
    }
}

MeshHandle ModelLoader::processMesh(aiMesh* mesh, Renderer& renderer)
{
    // extract vertices and indices
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

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

		vertices.push_back(vertex);
	}

	// Indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	return renderer.createMesh({ vertices, indices });
}

MaterialHandle ModelLoader::loadMaterial(aiMaterial* aiMat, const std::string& directory, Renderer& renderer)
{
	MaterialData mat{};

	aiColor3D color(0.f, 0.f, 0.f);
	if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
		mat.albedo = glm::vec3(color.r, color.g, color.b);
	}

	float value = 0.0f;
	if (aiMat->Get(AI_MATKEY_REFLECTIVITY, value) == AI_SUCCESS) {
		mat.metallic = value;
	}
	if (aiMat->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS) {
		mat.roughness = value;
	}

	aiString path;

	auto tryLoadTexture = [&renderer, &directory, aiMat, &path](aiTextureType type, TextureHandle& targetMap) {
		if (aiMat->GetTexture(type, 0, &path) == AI_SUCCESS) {
			std::string fullPath = directory + "/" + path.C_Str();
			targetMap = renderer.createTextureFromFile(fullPath);
		}
		};

	tryLoadTexture(aiTextureType_DIFFUSE, mat.albedoMap);
	tryLoadTexture(aiTextureType_NORMALS, mat.normalMap);
	tryLoadTexture(aiTextureType_SPECULAR, mat.metallicMap);
	tryLoadTexture(aiTextureType_SHININESS, mat.roughnessMap);
	tryLoadTexture(aiTextureType_AMBIENT, mat.aoMap);
	tryLoadTexture(aiTextureType_EMISSIVE, mat.emissiveMap);

    return renderer.createMaterial(mat);
}
