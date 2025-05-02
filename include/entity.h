#pragma once

#include "mesh.h"
#include "material.h"

#include <memory>

class Entity {
public:
	Entity();
	Entity(std::shared_ptr<Mesh> mesh, Material material, glm::vec3 position, std::string name = "new entity") : m_mesh(mesh), m_material(material), position(position), useMaterial(true), m_name(name) {
		/*if (material.shader) {
			material.shader->bind();
			material.shader->setUniformBool("useAlbedoTexture", material.useAlbedoMap);
			material.shader->setUniformBool("useNormalTexture", material.useNormalMap);
			material.shader->setUniformBool("useMetallicTexture", material.useMetalMap);
			material.shader->setUniformBool("useRoughnessTexture", material.useRoughMap);
			material.shader->setUniformBool("useAoTexture", material.useAoMap);
		}*/
	}

	void draw(const glm::mat4& view, const glm::mat4& projection);

	void drawMesh() {
		if (m_mesh) {

			m_mesh->draw();
		}
	}

	glm::mat4 getModelMatrix();

	void setMesh(std::shared_ptr<Mesh> mesh) { m_mesh = mesh; }

	void setMaterial(Material material) { m_material = material; }
	Material& getMaterial() { return m_material; }

	void setName(std::string name) { m_name = name; }
	std::string getName() { return m_name; }

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

private:
	std::shared_ptr<Mesh> m_mesh;
	Material m_material;

	std::shared_ptr<Shader> m_basicShader;

	bool useMaterial = false;

	std::string m_name;
};