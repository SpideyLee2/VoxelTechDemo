#pragma once

#include <glad/glad.h>

class VAO {
public:
	VAO(const float* data, const int& size);
	~VAO();

	void bind() const;
	void unbind() const;

	unsigned int getId() const;

private:
	unsigned int id;
};