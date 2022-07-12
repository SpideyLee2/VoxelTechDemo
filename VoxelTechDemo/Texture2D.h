#pragma once

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <iostream>

//enum class TEXTURE_TYPE {
//	DIFFUSE,
//	SPECULAR,
//	NORMAL
//};

enum class TEXTURE_TYPE {
	Air = 0,
	GrassTop,
	GrassSide,
	Dirt,
	Stone,
	Sand,
	Water,
	FullSpecular,
	NoSpecular
};

class Texture2D {
public:
	GLuint id;
	Texture2D() = default;
	Texture2D(std::string dir, bool adjustForGamma = false);
	~Texture2D();
	Texture2D(const Texture2D& tex2d); // copy constructor
	Texture2D(Texture2D&& tex2d) noexcept; // move constructor

	Texture2D& operator=(Texture2D&& tex2d) noexcept;

	void bind(const int &texUnit) const;
	void unbind() const;

	std::string getTexDir() const;
private:
	std::string directory;
	bool wasAdjustedForGamma;
	int width, height, numChannels;
	mutable int boundTexUnit;
};