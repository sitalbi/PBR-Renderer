#pragma once

#include "glad/glad.h"
#include <string>

enum class TextureType {
	ALBEDO,
	NORMAL,
	METALLIC,
	ROUGHNESS,
	AO
};

class Texture {

public:
	Texture();
	Texture(std::string path);
	~Texture();

	void load(const char* path);
	void bind(unsigned int slot = 0) const;
	void unbind() const;

private:

	unsigned int m_id;
	TextureType m_type;

	bool m_isNormalMap = false;
};