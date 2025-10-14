#include "entity.h"
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

Entity::Entity()
{
}

void Entity::draw(std::shared_ptr<Shader> shader, const glm::mat4& view, const glm::mat4& projection)
{
	if (!m_model) {
		return;
	}


	glm::mat4 modelMatrix = getModelMatrix();

	shader->bind();
	shader->setUniformMat4f("model", modelMatrix);
	shader->setUniformMat4f("view", view);
	shader->setUniformMat4f("projection", projection);

	if (pointLight) {
		if (!pointLight->hasMesh) {
			return;
		}
		if (materialOverrides.find(0) == materialOverrides.end()) {
			Material lightMat;
			lightMat.shader = shader;
			lightMat.emissiveColor = pointLight->color * pointLight->intensity;
			materialOverrides[0] = lightMat;
		}
		else {
			materialOverrides[0].emissiveColor = pointLight->color * pointLight->intensity;
		}
	}
	auto& submeshes = m_model->getSubmeshes();
	for (size_t i = 0; i < submeshes.size(); ++i) {
		Material* mat = materialOverrides.find(i) != materialOverrides.end()
			? &materialOverrides[i]
			: submeshes[i].material.get();

			mat->bind();
			submeshes[i].draw();
	}
	
	
	
}

glm::mat4 Entity::getModelMatrix()
{

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	modelMatrix = glm::scale(modelMatrix, scale);
	return modelMatrix;
}
