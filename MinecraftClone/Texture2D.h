#pragma once

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <iostream>

class Texture2D {
public:
	GLuint id;

	Texture2D(std::string dir, int unit);

	void bind();
	void unbind();
	int getTexUnit();

private:
	int texUnit;
};