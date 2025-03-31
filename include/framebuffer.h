#pragma once

#include <vector>

class Framebuffer
{

public:

	unsigned int fbo;
	unsigned int rbo;

	int width;
	int height;

	std::vector<unsigned int> textures;
	std::vector<unsigned int> attachments;

	Framebuffer(int width, int height);

	~Framebuffer();

	void clear();

	void bind();

	void unbind();

	bool isComplete();

	void createColorAttachment();

	void addColorAttachment(unsigned int textureID);

	void addDepthAttachment();

	void setDrawBuffers();


private:

};