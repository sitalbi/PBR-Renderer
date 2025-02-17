#pragma once

#include "mesh.h"
#include "material.h"

#include <memory>

class Entity {
public:
	Entity();
	// Constructor for entity with a basic shader and a mesh
	Entity(std::shared_ptr<Shader> basicShader, std::shared_ptr<Mesh> mesh) : m_basicShader(basicShader), m_mesh(mesh) {}
	// Constructor for entity with a mesh and a material (PBR)
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : m_mesh(mesh), m_material(material) {}

	void draw(const glm::mat4& view, const glm::mat4& projection);

	void setMesh(std::shared_ptr<Mesh> mesh) { m_mesh = mesh; }
	void setMaterial(std::shared_ptr<Material> material) { m_material = material; }

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

private:
	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<Material> m_material;

	std::shared_ptr<Shader> m_basicShader;
};