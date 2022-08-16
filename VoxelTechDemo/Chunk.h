#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite/FastNoiseLite.h>

#include <future>

#include "Block.h"
#include "Shader.h"

//static std::unordered_map<TEXTURE_TYPE, const std::shared_ptr<Texture2D>>* blockTextures;

/*
const static int8_t X_DIMENSION = 16;
const static int16_t Y_DIMENSION = 256;
const static int8_t Z_DIMENSION = 16;
const static int32_t VOLUME = X_DIMENSION * Y_DIMENSION * Z_DIMENSION;
const static int8_t MIN_Y = -128;
const static int8_t MAX_Y = 127;

const int8_t CUBE_OFFSET = 10;
*/

static int s_ChunkIDCounter = 0;

class Chunk {
public:
	Chunk(const glm::vec3& worldPos);
	Chunk(const Chunk& chunk) = delete; // Don't want to copy chunks
	Chunk(Chunk&& chunk) noexcept;
	Chunk() = default;
	~Chunk() = default;

	Chunk& operator=(const Chunk& chunk) = delete; // Don't want to copy chunks
	Chunk& operator=(Chunk&& chunk) noexcept;

	void render(const Shader& shader) const;
	void generateVAOandVBO();
	void populateVBO();
	int getID();

	unsigned int m_Vao;
	unsigned int m_Vbo;
	glm::vec3 m_WorldPos;
	glm::mat4 m_Model;
	std::unique_ptr<Block[]> m_pBlocks;

	int m_NumVisFaces = 0;
	std::vector<Vertex> m_VisVerts;

	const static int X_DIMENSION = 32;
	const static int Y_DIMENSION = 32;
	const static int Z_DIMENSION = 32;
	const static int VOLUME = X_DIMENSION * Y_DIMENSION * Z_DIMENSION;
	const static int CUBE_OFFSET = 10;
private:
	//void generate();
	//void updateVFM(const int& x, const int& y, const int& z);

	int m_ID;
	//int m_NumVisFaces;
};

