#include "VAO.h"

VAO::VAO() {
	glGenVertexArrays(1, &id);
}

VAO::~VAO() {
	glDeleteBuffers(1, &id);
}

unsigned int VAO::getId() const {
	return id;
}

void VAO::bind() const {
	glBindVertexArray(id);
}

void VAO::unbind() const {
	glBindVertexArray(0);
}