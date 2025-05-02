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

	unsigned int depthTexture;

	Framebuffer(int width, int height);

	~Framebuffer();

	void clear();

	void bind();

	void unbind();

	bool isComplete();

	void createColorAttachment();

	void addColorAttachment(unsigned int textureID);

	void addDepthRenderBuffer();
	void addDepthTexture();
	void setDepthTexture(unsigned int textureID, int attachement);

	void setDrawBuffers();


private:

};