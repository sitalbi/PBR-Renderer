#include "framebuffer.h"
#include "glad/glad.h"

Framebuffer::Framebuffer(int width, int height) : width(width), height(height)
{
	glGenFramebuffers(1, &fbo);
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &fbo);
	if (rbo)
	{
		glDeleteRenderbuffers(1, &rbo);
	}
	for (auto& texture : textures)
	{
		glDeleteTextures(1, &texture);
	}
}

void Framebuffer::clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::isComplete()
{
	bind();
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	unbind();
	return status == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::createColorAttachment()
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	textures.push_back(texture);

	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + textures.size() - 1, GL_TEXTURE_2D, texture, 0);
	unbind();
}

void Framebuffer::addColorAttachment(unsigned int textureID)
{
	textures.push_back(textureID);
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + textures.size() - 1, GL_TEXTURE_2D, textureID, 0);
	unbind();
}

void Framebuffer::addDepthAttachment()
{
	bind();
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	unbind();
}

void Framebuffer::setDrawBuffers()
{
	for (unsigned int i = 0; i < textures.size(); ++i) {
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffers(attachments.size(), attachments.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

