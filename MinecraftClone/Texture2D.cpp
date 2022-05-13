#include "Texture2D.h"

Texture2D::Texture2D(std::string dir, int unit) : texUnit(unit) {
	std::string imageType = dir.substr(dir.size() - 4);

	glGenTextures(1, &id);
	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, numChannels;
	unsigned char* texData = stbi_load(dir.c_str(), &width, &height, &numChannels, 0);

	if (texData) {
		if (imageType == ".png") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
		}
		else if (imageType == ".jpg") {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
		}
		else {
			std::cout << "Please add support to the Texture2D class for loading images of type '" << imageType << "'.\n";
		}

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load 2D texture data at: " << dir << std::endl;
	}

	unbind();
	stbi_image_free(texData);
}

void Texture2D::bind() {
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture2D::unbind() {
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int Texture2D::getTexUnit() {
	return texUnit;
}