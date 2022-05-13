#include "Block.h"

Block::Block() {

}

Block::Block(GLfloat* verts, GLuint tex, glm::vec3 worldPosition, glm::vec3 gradientVector) 
	: vertices(verts), texture(tex), worldPos(worldPosition), gradVector(gradientVector) {
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
