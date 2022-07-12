#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite/FastNoiseLite.h>

#include "Block.h"
#include "Shader.h"

const static int X_DIMENSION = 16;
const static int Y_DIMENSION = 256;
const static int Z_DIMENSION = 16;
const static int VOLUME = X_DIMENSION * Y_DIMENSION * Z_DIMENSION;
const static int MIN_Y = 0;
const static int MAX_Y = 255;

const int CUBE_OFFSET = 10;

static std::unique_ptr<FastNoiseLite> noise;

static std::unordered_map<TEXTURE_TYPE, const std::shared_ptr<Texture2D>>* blockTextures;

/*
const static int8_t X_DIMENSION = 16;
const static int16_t Y_DIMENSION = 256;
const static int8_t Z_DIMENSION = 16;
const static int32_t VOLUME = X_DIMENSION * Y_DIMENSION * Z_DIMENSION;
const static int8_t MIN_Y = -128;
const static int8_t MAX_Y = 127;

const int8_t CUBE_OFFSET = 10;
*/

static int s_chunkID = 0;

class Chunk {
public:
	Chunk(glm::vec2&& worldPos);
	Chunk(const Chunk& chunk) = default;
	Chunk(Chunk&& chunk) = default;

	Block& getBlockAt(glm::ivec3 blockPosInChunk) const;
	void render(const Shader& shader) const;

	glm::vec2 m_worldPos;
	std::unique_ptr<Block[]> m_pBlocks;
private:
	void generate();
	
	const int m_ID;
};

