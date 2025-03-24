#include "texture.h"
#include <iostream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



Texture::Texture()
{
}

Texture::Texture(std::string path)
{
	load(path.c_str());
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_id);
}

void Texture::load(const char* path)
{
    std::string filename = path;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    
    if (filename.find("albedo") != std::string::npos || filename.find("alb") != std::string::npos ||
        filename.find("diffuse") != std::string::npos || filename.find("color") != std::string::npos) {
        m_type = TextureType::ALBEDO;
    }
    else if (filename.find("normal") != std::string::npos || filename.find("norm") != std::string::npos ||
        filename.find("nrm") != std::string::npos) {
        m_type = TextureType::NORMAL;
    }
    else if (filename.find("metallic") != std::string::npos || filename.find("metal") != std::string::npos) {
        m_type = TextureType::METALLIC;
    }
    else if (filename.find("roughness") != std::string::npos || filename.find("rough") != std::string::npos) {
        m_type = TextureType::ROUGHNESS;
    }
    else if (filename.find("ao") != std::string::npos || filename.find("ambient") != std::string::npos ||
        filename.find("occlusion") != std::string::npos) {
        m_type = TextureType::AO;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image data
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);  // Flip Y-axis during loading
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        // Different handling based on texture type and channel count
        if (m_type == TextureType::ALBEDO) {
            // Color textures should use sRGB
            if (nrChannels == 3)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            else if (nrChannels == 4)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else {
            // Non-color textures should use linear space
            if (nrChannels == 1)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
            else if (nrChannels == 3)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            else if (nrChannels == 4)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }

    stbi_image_free(data);
    unbind();
}

void Texture::bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

