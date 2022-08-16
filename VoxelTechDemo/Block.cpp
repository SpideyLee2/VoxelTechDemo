#include "Block.h"

Block::Block(const BLOCK_TYPE& blockType, const TEXTURE_TYPE& texType, const glm::mat4& modelMatrix, const int& chunkID, 
			 const glm::ivec3& chunkPos)
	: m_BlockType(blockType),
	  m_TexType(texType),
	  m_Model(modelMatrix), 
	  m_WorldPos(glm::vec3(modelMatrix[3])),
	  m_ChunkID(chunkID), 
	  m_ChunkPos(chunkPos) {

	blockType == BLOCK_TYPE::Air ? m_IsSolid = false : m_IsSolid = true;
	
	//m_AdjBlocks = std::array<BLOCK_TYPE, 6>();
}

BLOCK_TYPE Block::getBlockType() const {
	return m_BlockType;
}

TEXTURE_TYPE Block::getTexType() const {
	return m_TexType;
}

int Block::getChunkID() const {
	return m_ChunkID;
}

glm::vec3 Block::getChunkPos() const {
	return m_ChunkPos;
}

glm::vec3 Block::getWorldPos() const {
	return m_WorldPos;
}

//void Block::setAdjBlockType(const BLOCK_TYPE blockType, int pos) {
//	m_AdjBlocks[pos] = blockType;
//}

Block::Block(const Block& block) : 
	m_Model(block.m_Model),
	m_BlockType(block.m_BlockType),
	m_ChunkID(block.m_ChunkID),
	m_ChunkPos(block.m_ChunkPos),
	m_WorldPos(block.m_WorldPos),
	m_TexType(block.m_TexType),
	m_IsSolid(block.m_IsSolid)/*, m_AdjBlocks(block.m_AdjBlocks)*/ {
}

Block::Block(Block&& block) noexcept : 
	m_Model(std::move(block.m_Model)),
	m_BlockType(std::move(block.m_BlockType)),
	m_ChunkID(std::move(block.m_ChunkID)),
	m_ChunkPos(std::move(block.m_ChunkPos)),
	m_WorldPos(std::move(block.m_WorldPos)),
	m_TexType(std::move(block.m_TexType)),
	m_IsSolid(std::move(block.m_IsSolid))/*, m_AdjBlocks(std::move(block.m_AdjBlocks))*/ {
}

Block& Block::operator=(Block&& block) noexcept {
	if (this != &block) {
		m_Model = std::move(block.m_Model);
		m_BlockType = std::move(block.m_BlockType);
		m_ChunkID = std::move(block.m_ChunkID);
		m_ChunkPos = std::move(block.m_ChunkPos);
		m_WorldPos = std::move(block.m_WorldPos);
		m_TexType = std::move(block.m_TexType);
		m_IsSolid = std::move(block.m_IsSolid);
		//m_AdjBlocks = std::move(block.m_AdjBlocks);
	}

	return *this;
}