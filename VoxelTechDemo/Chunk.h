#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite/FastNoiseLite.h>

#include <future>

#include "Block.h"
#include "Shader.h"

static std::unique_ptr<FastNoiseLite> noise;

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
	Chunk(glm::vec2&& worldPos, const glm::vec3& cameraPos);
	Chunk(const Chunk& chunk) = delete;
	Chunk(Chunk&& chunk) noexcept;
	Chunk() = default;

	Chunk& operator=(const Chunk& chunk) = delete;
	Chunk& operator=(Chunk&& chunk) noexcept;

	Block& getBlockAt(const glm::ivec3& blockPosInChunk) const;
	void render(const Shader& shader, const glm::vec3& cameraPos, const bool& cameraHasMoved);
	void generateVAOandVBO();
	void updateVisibleFacesMesh(const glm::vec3& cameraPos);
	void populateVBO();

	unsigned int m_Vao;
	unsigned int m_Vbo;
	glm::vec2 m_WorldPos;
	std::unique_ptr<Block[]> m_pBlocks;

	const static int X_DIMENSION = 64;
	const static int Y_DIMENSION = 64;
	const static int Z_DIMENSION = 64;
	const static int VOLUME = X_DIMENSION * Y_DIMENSION * Z_DIMENSION;
	const static int CUBE_OFFSET = 10;
	int m_NumVisFaces;
private:
	void generate();
	void updateVFM(const int& x, const int& y, const int& z);

	int m_ID;
	//int m_NumVisFaces;
	std::vector<Vertex> m_VisVerts;
};

