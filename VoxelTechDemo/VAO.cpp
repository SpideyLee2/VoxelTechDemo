#include "VAO.h"

VAO::VAO(const float* data, const int& size) {
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data, GL_STATIC_DRAW);

	// TODO: This is hard-coded. Make it more modular (if necessary). =======================
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	// ========================================================================

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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