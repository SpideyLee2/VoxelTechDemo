#pragma once

#include "Texture2D.h"
#include "VAO.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <unordered_map>
#include <array>

enum class BLOCK_TYPE {
	Air = 0,
	Grass,
	Dirt,
	Stone,
	Sand,
	Water
};

// Vertex data of a cube with CCW winding order (position, normal, tex coord)
const static float s_data[] = {
	// POSITION				// NORMAL				// TEX COORDS
	// Back face
	-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 0.0f, // bottom-left
	 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 1.0f, // top-right
	-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 0.0f, // bottom-left
	-0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 1.0f, // top-left
	// Front face
	-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 1.0f, // top-right
	-0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 1.0f, // top-left
	-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 0.0f, // bottom-left
	// Left face
	-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
	-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
	-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
	-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
	// Right face
	 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
	 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
	// Bottom face
	-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 0.0f, // bottom-right
	-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 1.0f, // top-right
	// Top face
	-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f, // top-left
	 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f, // top-left
	-0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 0.0f  // bottom-left
};

//const static float s_DataBackFace[] = {
//	// POSITION				// NORMAL				// TEX COORDS
//	// Back face
//	-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 0.0f, // bottom-left
//	 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 1.0f, // top-right
//	 0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 0.0f, // bottom-right
//	 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 1.0f, // top-right
//	-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 0.0f, // bottom-left
//	-0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 1.0f, // top-left
//};
//const static float s_DataFrontFace[] = {
//	// POSITION				// NORMAL				// TEX COORDS
//	// Front face
//	-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 0.0f, // bottom-left
//	 0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 0.0f, // bottom-right
//	 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 1.0f, // top-right
//	 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 1.0f, // top-right
//	-0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 1.0f, // top-left
//	-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 0.0f, // bottom-left
//};
//const static float s_DataLeftFace[] = {
//	// POSITION				// NORMAL				// TEX COORDS
//	// Left face
//	-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
//	-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
//	-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
//	-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
//	-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
//	-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
//};
//const static float s_DataRightFace[] = {
//	// POSITION				// NORMAL				// TEX COORDS
//	// Right face
//	 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
//	 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
//	 0.5f,  0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
//	 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
//	 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
//	 0.5f, -0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
//};
//const static float s_DataBottomFace[] = {
//	// POSITION				// NORMAL				// TEX COORDS
//	// Bottom face
//	-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 1.0f, // top-right
//	 0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 1.0f, // top-left
//	 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 0.0f, // bottom-left
//	 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 0.0f, // bottom-left
//	-0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 0.0f, // bottom-right
//	-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 1.0f, // top-right
//};
//const static float s_DataTopFace[] = {
//	// POSITION				// NORMAL				// TEX COORDS
//	// Top face
//	-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f, // top-left
//	 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
//	 0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 1.0f, // top-right
//	 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
//	-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f, // top-left
//	-0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 0.0f  // bottom-left
//};

static std::unique_ptr<VAO> s_vao;

class Block {
public:
	Block() = default; // Used in data structures as a placeholder for real blocks
	Block(const BLOCK_TYPE& blockType, const std::shared_ptr<Texture2D> texture, 
		  const glm::mat4& modelMatrix, const int& chunkID, const glm::ivec3& chunkPos);
	~Block() = default;
	Block(const Block& block); // copy ctor
	Block(Block&& block) noexcept; // move ctor

	Block& operator=(Block&& block) noexcept; // move assignment operator

	static void bindVAO();
	static void unbindVAO();
	void bindTextures() const;
	void unbindTextures() const;
	//void renderFace(int pos); // 0 = back, 1 = front, 2 = left, 3 = right, 4 = bottom, 5 = top

	BLOCK_TYPE getBlockType() const;
	int getChunkID() const;
	glm::vec3 getChunkPos() const;
	glm::vec3 getWorldPos() const;
	//void setAdjBlockType(const BLOCK_TYPE blockType, int pos);

	glm::mat4 m_model;
	//std::array<BLOCK_TYPE, 6> m_AdjBlocks;
private:
	BLOCK_TYPE m_blockType;
	int m_chunkID;
	glm::ivec3 m_chunkPos;
	glm::vec3 m_worldPos;

	std::shared_ptr<Texture2D> m_pDiffuseTexture;
};