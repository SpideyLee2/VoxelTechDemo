#pragma once

#include "Chunk2.h"
#include <glm/gtx/hash.hpp>
#include <future>

static std::unique_ptr<FastNoiseLite> noise;

// This could be implemented as a singleton or with all static members, but it's not necessary at the moment.
class ChunkManager {
public:
	ChunkManager(const glm::vec3& playerPos);
	ChunkManager() = default;
	~ChunkManager() = default;

	void renderChunks(const Shader& shader);
	void renderChunkCenters(const Shader& shader, const unsigned int& vao); // for debugging
	void recalculate(const glm::vec3& playerPos);

	std::unordered_map<glm::vec3, Chunk2, std::hash<glm::vec3>> m_Chunks;

private:
	//void loadChunk(const glm::vec3& worldPos);
	Chunk2 loadChunk(glm::vec3&& worldPos);
	void unloadChunk(const glm::vec3& worldPos);
	int roundPosToNearestChunk(const float& pos, const int& dim) const;
	void updateVisibleVertices(Chunk2& chunk);

	int m_ChunkRenderRadius = 8;
	static const int NUM_CHUNKS_PER_STACK = 256 / Chunk2::Y_DIMENSION;

	int m_LastRoundedPlayerPosX = 0;
	int m_LastRoundedPlayerPosZ = 0;
};