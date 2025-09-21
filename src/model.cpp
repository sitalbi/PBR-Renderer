#include "model.h"
#include <iostream>
#include <filesystem>


std::shared_ptr<Material> Model::defaultMaterial = std::make_shared<Material>();

Model::Model()
{
}

Model::~Model()
{
}

void Model::loadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
		return;
	}

	submeshes.clear();

	std::filesystem::path filepath(path);
	std::string directory = filepath.parent_path().string();

	processNode(scene->mRootNode, scene, directory);
}

void Model::loadSphere(float radius, unsigned int segments)
{
	submeshes.clear();
	Mesh sphereMesh;
	sphereMesh.loadSphere(radius, segments);
	sphereMesh.material = defaultMaterial;
	submeshes.push_back(sphereMesh);
}

void Model::loadCube(float size)
{
	submeshes.clear();
	Mesh cubeMesh;
	cubeMesh.loadCube(size);
	cubeMesh.material = defaultMaterial;
	submeshes.push_back(cubeMesh);
}

void Model::draw()
{
	for (auto& mesh : submeshes) {
		mesh.material->bind();
		mesh.draw();
	}
}

void Model::processNode(aiNode* node, const aiScene* scene, const std::string& directory)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];

		Mesh submesh = processMesh(mesh);
		submesh.material = loadMaterial(aiMat, directory); 
		submeshes.push_back(submesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene, directory);
	}
}

Mesh Model::processMesh(aiMesh* mesh)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;

		vertex.m_position = glm::vec3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		);

		vertex.m_normal = mesh->HasNormals() ?
			glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
			: glm::vec3(0.0f);

		vertex.m_texCoords = mesh->mTextureCoords[0] ?
			glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
			: glm::vec2(0.0f);

		vertex.m_tangent = mesh->HasTangentsAndBitangents() ?
			glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z)
			: glm::vec3(0.0f);

		vertex.m_bitangent = mesh->HasTangentsAndBitangents() ?
			glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z)
			: glm::vec3(0.0f);

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh result(vertices, indices);
	result.setupMesh();
	return result;
}

std::shared_ptr<Material> Model::loadMaterial(aiMaterial* aiMat, const std::string& directory)
{
	Material mat;
	mat.shader = defaultMaterial ? defaultMaterial->shader : nullptr;

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

	auto tryLoadTexture = [&](aiTextureType type, std::shared_ptr<Texture>& targetMap, bool& flag) {
		if (aiMat->GetTexture(type, 0, &path) == AI_SUCCESS) {
			std::string fullPath = directory + "/" + path.C_Str();
			targetMap = std::make_shared<Texture>(fullPath);
			flag = true;
		}
		};

	tryLoadTexture(aiTextureType_DIFFUSE, mat.albedoMap, mat.useAlbedoMap);
	tryLoadTexture(aiTextureType_NORMALS, mat.normalMap, mat.useNormalMap);
	tryLoadTexture(aiTextureType_SPECULAR, mat.metallicMap, mat.useMetalMap);
	tryLoadTexture(aiTextureType_SHININESS, mat.roughnessMap, mat.useRoughMap);
	tryLoadTexture(aiTextureType_AMBIENT, mat.aoMap, mat.useAoMap);
	tryLoadTexture(aiTextureType_EMISSIVE, mat.emissiveMap, mat.useEmissiveMap);

	return std::make_shared<Material>(mat);
}
