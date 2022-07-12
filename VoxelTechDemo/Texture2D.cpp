#include "Texture2D.h"

Texture2D::Texture2D(std::string dir, bool adjustForGamma) : directory(dir), wasAdjustedForGamma(adjustForGamma), boundTexUnit(-1) {
	//std::string imageType = dir.substr(dir.size() - 4);

	glGenTextures(1, &id);
	bind(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned char* texData = stbi_load(dir.c_str(), &width, &height, &numChannels, 0);

	if (texData) {
		switch (numChannels) {
			case 1:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texData);
				break;
			case 2:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, texData);
				break;
			case 3:
				if (adjustForGamma)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_SRGB, GL_UNSIGNED_BYTE, texData);
				else
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
				break;
			case 4:
				if (adjustForGamma)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_SRGB_ALPHA, GL_UNSIGNED_BYTE, texData);
				else
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
				break;
			default:
				std::cout << "Number of texture color channels exceeds supported range [1,4].\n";
		}

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load 2D texture data at: " << dir << std::endl;
	}

	unbind();
	stbi_image_free(texData);
}

void Texture2D::bind(const int& texUnit) const {
	//std::cout << "Binding texture: " << directory << std::endl;
	boundTexUnit = texUnit;
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture2D::unbind() const {
	if (boundTexUnit < 0) {
		std::cout << "Texture " << id << " is not bound to any texture unit. It cannot be unbound.\n";
	}
	glActiveTexture(GL_TEXTURE0 + boundTexUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
	boundTexUnit = -1;
}

std::string Texture2D::getTexDir() const {
	return directory;
}

// Copy constructor
Texture2D::Texture2D(const Texture2D& tex2d)
	: id(tex2d.id),
	  boundTexUnit(tex2d.boundTexUnit),
	  directory(tex2d.directory),
	  wasAdjustedForGamma(tex2d.wasAdjustedForGamma),
	  width(tex2d.width),
	  height(tex2d.height),
	  numChannels(tex2d.numChannels) {}

// Move constructor
Texture2D::Texture2D(Texture2D&& tex2d) noexcept
	: id(std::move(tex2d.id)),
	  boundTexUnit(std::move(tex2d.boundTexUnit)),
	  directory(std::move(tex2d.directory)),
	  wasAdjustedForGamma(std::move(tex2d.wasAdjustedForGamma)),
	  width(std::move(tex2d.width)),
	  height(std::move(tex2d.height)),
	  numChannels(std::move(tex2d.numChannels)) {
}

Texture2D::~Texture2D() {
	std::cout << "Deleting texture generated from " << directory << std::endl;
	glDeleteTextures(1, &id);
}

Texture2D& Texture2D::operator=(Texture2D&& tex2d) noexcept {
	if (this != &tex2d) {
		id = std::move(tex2d.id);
		boundTexUnit = std::move(tex2d.boundTexUnit);
		directory = std::move(tex2d.directory);
		wasAdjustedForGamma = std::move(tex2d.wasAdjustedForGamma);
		width = std::move(tex2d.width);
		height = std::move(tex2d.height);
		numChannels = std::move(tex2d.numChannels);
	}
	return *this;
}