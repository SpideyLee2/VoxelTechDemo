#pragma once

#include "Chunk2.h"
#include <glm/gtx/hash.hpp>

static std::unique_ptr<FastNoiseLite> noise;

class ChunkManager {
public:
	ChunkManager(const glm::vec3& playerPos);

	void updateVisibleFacesMesh(Chunk2& chunk);
	void renderChunks(const Shader& shader);
	void renderChunkCenters(const Shader& shader, const unsigned int& vao); // for debugging

	std::unordered_map<glm::vec3, Chunk2, std::hash<glm::vec3>> m_Chunks;

private:
	int roundPosToNearestChunk(const float& pos) const;
	void loadChunk(const glm::vec3& worldPos);
	void unloadChunk(Chunk2& chunk);

	int m_ChunkRenderRadius = 3;
	int m_NumChunksPerStack = 256 / Chunk2::Y_DIMENSION;
};