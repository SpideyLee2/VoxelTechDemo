#include "Block.h"

Block::Block(const BLOCK_TYPE& blockType, const std::shared_ptr<Texture2D> texture, 
			 const glm::mat4& modelMatrix, const int& chunkID, 
			 const glm::ivec3& chunkPos)
	: m_blockType(blockType),
	  m_pDiffuseTexture(texture),
	  m_model(modelMatrix), 
	  m_worldPos(glm::vec3(modelMatrix[3])),
	  m_chunkID(chunkID), 
	  m_chunkPos(chunkPos) {

	if (s_vao == nullptr) {
		s_vao = std::make_unique<VAO>(s_data, 288);
	}
	
	//m_AdjBlocks = std::array<BLOCK_TYPE, 6>();
}

Block::Block(const Block& block) 
	: m_model(block.m_model), 
	  m_blockType(block.m_blockType),
	  m_chunkID(block.m_chunkID),
	  m_chunkPos(block.m_chunkPos),
	  m_worldPos(block.m_worldPos), 
	  m_pDiffuseTexture(block.m_pDiffuseTexture)/*, m_AdjBlocks(block.m_AdjBlocks)*/ {
}

Block::Block(Block&& block) noexcept
	: m_model(std::move(block.m_model)),
	  m_blockType(std::move(block.m_blockType)),
	  m_chunkID(std::move(block.m_chunkID)),
	  m_chunkPos(std::move(block.m_chunkPos)),
	  m_worldPos(std::move(block.m_worldPos)),
	  m_pDiffuseTexture(std::move(block.m_pDiffuseTexture))/*, m_AdjBlocks(std::move(block.m_AdjBlocks))*/ {
}

Block& Block::operator=(Block&& block) noexcept {
	if (this != &block) {
		m_pDiffuseTexture = nullptr;

		m_model = std::move(block.m_model);
		m_blockType = std::move(block.m_blockType);
		m_chunkID = std::move(block.m_chunkID);
		m_chunkPos = std::move(block.m_chunkPos);
		m_worldPos = std::move(block.m_worldPos);

		m_pDiffuseTexture = std::move(block.m_pDiffuseTexture);
		//m_AdjBlocks = std::move(block.m_AdjBlocks);
	}

	return *this;
}

void Block::bindVAO() {
	s_vao->bind();
}

void Block::unbindVAO() {
	s_vao->unbind();
}

void Block::bindTextures() const {
	m_pDiffuseTexture->bind(0);
}

void Block::unbindTextures() const {
	m_pDiffuseTexture->unbind();
}

BLOCK_TYPE Block::getBlockType() const {
	return m_blockType;
}

int Block::getChunkID() const {
	return m_chunkID;
}

glm::vec3 Block::getChunkPos() const {
	return m_chunkPos;
}

glm::vec3 Block::getWorldPos() const {
	return m_worldPos;
}

//void Block::setAdjBlockType(const BLOCK_TYPE blockType, int pos) {
//	m_AdjBlocks[pos] = blockType;
//}