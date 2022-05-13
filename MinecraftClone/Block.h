#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class Block {
public:
	GLuint id;
	GLfloat* vertices;
	GLuint texture;
	glm::vec3 worldPos;
	
	glm::vec3 gradVector;

	Block();
	Block(GLfloat* verts, GLuint tex, glm::vec3 worldPosition, glm::vec3 gradientVector);
};

