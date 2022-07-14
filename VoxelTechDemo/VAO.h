#pragma once

#include <glad/glad.h>

class VAO {
public:
	VAO();
	~VAO();

	void bind() const;
	void unbind() const;

	unsigned int getId() const;

private:
	unsigned int id;
};