#pragma once

#include "glm/glm.hpp"

/*
	Vertex attributes
*/
struct Vertex {
    glm::vec3 m_position;
    glm::vec3 m_normal;
	glm::vec2 m_texCoords;
	glm::vec3 m_tangent;
	glm::vec3 m_bitangent;
};