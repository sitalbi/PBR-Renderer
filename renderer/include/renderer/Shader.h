#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

class Shader
{
private:
	unsigned int m_shaderID;
	std::string m_vertexFilePath;
	std::string m_fragmentFilePath;

	mutable std::unordered_map<std::string, int> m_UniformLocationCache; // Cache for uniforms

	unsigned int compile();
	int getUniformLocation(const std::string& name) const;

public:
	Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath);
	~Shader();

	void bind() const;
	void unbind() const;

	unsigned int getID() const { return m_shaderID; }

	// Set uniforms
	void setUniform1f(const std::string& name, float value) const;
	void setUniform2f(const std::string& name, float v0, float v1) const;
	void setUniform3f(const std::string& name, float v0, float v1, float v2) const;
	void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
	void setUniform1i(const std::string& name, int value) const;
	void setUniformMat3f(const std::string& name, const glm::mat3& matrix) const;
	void setUniformMat4f(const std::string& name, const glm::mat4& matrix) const;
	void setUniformVec3f(const std::string& name, const glm::vec3& vector) const;
	void setUniformBool(const std::string& name, bool value) const;
	void setUniform3fv(const std::string& name, const std::vector<glm::vec3>& vector, int count) const;
};