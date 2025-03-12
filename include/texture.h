#pragma once

#include "glad/glad.h"
#include <string>


class Texture {

public:
	Texture();
	Texture(std::string path, bool isNormalMap = false);
	~Texture();

	void load(const char* path);
	void bind(unsigned int slot = 0) const;
	void unbind() const;

private:

	unsigned int m_id;

	bool m_isNormalMap = false;
};