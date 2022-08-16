#pragma once

#include "Texture2D.h"
#include "VAO.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <unordered_map>
#include <array>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 texCoords;
};

enum class BLOCK_TYPE {
	Grass = 0,
	Dirt,
	Stone,
	Sand,
	Water,
	Air
};

enum class BLOCK_FACE {
	Back = 0,
	Front,
	Left,
	Right,
	Bottom,
	Top
};

//static std::unique_ptr<VAO> s_vao;

class Block {
public:
	Block() = default; // Used in data structures as a placeholder for real blocks
	Block(const BLOCK_TYPE& blockType, const TEXTURE_TYPE& texType, const glm::mat4& modelMatrix, 
		  const int& chunkID, const glm::ivec3& chunkPos);
	~Block() = default;
	Block(const Block& block); // copy ctor
	Block(Block&& block) noexcept; // move ctor

	Block& operator=(Block&& block) noexcept; // move assignment operator

	//void updateVisibleFaces(const std::vector<BLOCK_FACE>& visibleFaces);
	//static void bindVAO();
	//static void unbindVAO();
	//void bindTextures() const;
	//void unbindTextures() const;

	BLOCK_TYPE getBlockType() const;
	TEXTURE_TYPE getTexType() const;
	int getChunkID() const;
	glm::vec3 getChunkPos() const;
	glm::vec3 getWorldPos() const;

	//void setAdjBlockType(const BLOCK_TYPE blockType, int pos);

	// Vertex data of a cube with CCW winding order (position, normal, tex coord)
	//inline const static float s_Data[] = {
	//	// POSITION				// NORMAL				// TEX COORDS
	//	// Back face
	//	-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 0.0f, // bottom-left
	//	 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 1.0f, // top-right
	//	 0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 0.0f, // bottom-right
	//	 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 0.0f, 1.0f, // top-right
	//	-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 0.0f, // bottom-left
	//	-0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,	 1.0f, 1.0f, // top-left
	//	// Front face
	//	-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 0.0f, // bottom-left
	//	 0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 0.0f, // bottom-right
	//	 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 1.0f, // top-right
	//	 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 1.0f, 1.0f, // top-right
	//	-0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 1.0f, // top-left
	//	-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,	 0.0f, 0.0f, // bottom-left
	//	// Left face
	//	-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
	//	-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
	//	-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
	//	-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
	//	-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	//	-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
	//	// Right face
	//	 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
	//	 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	//	 0.5f,  0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 1.0f, // top-right
	//	 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	//	 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 1.0f, // top-left
	//	 0.5f, -0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	 0.0f, 0.0f, // bottom-left
	//	// Bottom face
	//	-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 1.0f, // top-right
	//	 0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 1.0f, // top-left
	//	 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 0.0f, // bottom-left
	//	 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 1.0f, 0.0f, // bottom-left
	//	-0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 0.0f, // bottom-right
	//	-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 1.0f, // top-right
	//	// Top face
	//	-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f, // top-left
	//	 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	//	 0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 1.0f, // top-right
	//	 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 1.0f, 0.0f, // bottom-right
	//	-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 1.0f, // top-left
	//	-0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,	 0.0f, 0.0f  // bottom-left
	//};

	//inline const static Vertex s_DataBackFace[] = {
	//	// POSITION								// NORMAL						// TEX COORDS
	//	Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(0.0f,  0.0f, -1.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-left
	//	Vertex{glm::vec3(0.5f,  0.5f, -0.5f),	glm::vec3(0.0f,  0.0f, -1.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(0.0f,  0.0f, -1.0f),	glm::vec3(0.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(0.5f,  0.5f, -0.5f),	glm::vec3(0.0f,  0.0f, -1.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(0.0f,  0.0f, -1.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-left
	//	Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),	glm::vec3(0.0f,  0.0f, -1.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}  // top-left
	//};
	//inline const static Vertex s_DataFrontFace[] = {
	//	// POSITION								// NORMAL						// TEX COORDS
	//	Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),	glm::vec3(0.0f,  0.0f, 1.0f),	glm::vec3(0.0f, 0.0f, 0.0f)}, // bottom-left
	//	Vertex{glm::vec3(0.5f, -0.5f,  0.5f),	glm::vec3(0.0f,  0.0f, 1.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(0.5f,  0.5f,  0.5f),	glm::vec3(0.0f,  0.0f, 1.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(0.5f,  0.5f,  0.5f),	glm::vec3(0.0f,  0.0f, 1.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),	glm::vec3(0.0f,  0.0f, 1.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-left
	//	Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),	glm::vec3(0.0f,  0.0f, 1.0f),	glm::vec3(0.0f, 0.0f, 0.0f)}  // bottom-left
	//};
	//inline const static Vertex s_DataLeftFace[] = {
	//	// POSITION								// NORMAL						// TEX COORDS
	//	Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),	glm::vec3(-1.0f, 0.0f, 0.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),	glm::vec3(-1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-left
	//	Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(-1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 0.0f)}, // bottom-left
	//	Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(-1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 0.0f)}, // bottom-left
	//	Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),	glm::vec3(-1.0f, 0.0f, 0.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),	glm::vec3(-1.0f, 0.0f, 0.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}  // top-right
	//};
	//inline const static Vertex s_DataRightFace[] = {
	//	// POSITION								// NORMAL						// TEX COORDS
	//	Vertex{glm::vec3(0.5f,  0.5f,  0.5f),	glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-left
	//	Vertex{glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(0.5f,  0.5f, -0.5f),	glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(0.5f,  0.5f,  0.5f),	glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-left
	//	Vertex{glm::vec3(0.5f, -0.5f,  0.5f),	glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 0.0f)}  // bottom-left
	//};
	//inline const static Vertex s_DataBottomFace[] = {
	//	// POSITION								// NORMAL						// TEX COORDS
	//	Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(0.0f, -1.0f, 0.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(0.0f, -1.0f, 0.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}, // top-left
	//	Vertex{glm::vec3(0.5f, -0.5f,  0.5f),	glm::vec3(0.0f, -1.0f, 0.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-left
	//	Vertex{glm::vec3(0.5f, -0.5f,  0.5f),	glm::vec3(0.0f, -1.0f, 0.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-left
	//	Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),	glm::vec3(0.0f, -1.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(0.0f, -1.0f, 0.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}  // top-right
	//};
	//inline const static Vertex s_DataTopFace[] = {
	//	// POSITION								// NORMAL						// TEX COORDS
	//	Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),	glm::vec3(0.0f,  1.0f,  0.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-left
	//	Vertex{glm::vec3(0.5f,  0.5f,  0.5f),	glm::vec3(0.0f,  1.0f,  0.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(0.5f,  0.5f, -0.5f),	glm::vec3(0.0f,  1.0f,  0.0f),	glm::vec3(1.0f, 1.0f, 0.0f)}, // top-right
	//	Vertex{glm::vec3(0.5f,  0.5f,  0.5f),	glm::vec3(0.0f,  1.0f,  0.0f),	glm::vec3(1.0f, 0.0f, 0.0f)}, // bottom-right
	//	Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),	glm::vec3(0.0f,  1.0f,  0.0f),	glm::vec3(0.0f, 1.0f, 0.0f)}, // top-left
	//	Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),	glm::vec3(0.0f,  1.0f,  0.0f),	glm::vec3(0.0f, 0.0f, 0.0f)} // bottom-left
	//};
	
	const static int VERTS_PER_FACE = 6;
	const static int ELEMS_PER_VERT = 9;

	//std::vector<BLOCK_FACE> visFaces;

	bool m_IsSolid;
	glm::mat4 m_Model;
	//std::array<BLOCK_TYPE, 6> m_AdjBlocks;
private:
	BLOCK_TYPE m_BlockType;
	TEXTURE_TYPE m_TexType;
	int m_ChunkID;
	glm::ivec3 m_ChunkPos;
	glm::vec3 m_WorldPos;
};