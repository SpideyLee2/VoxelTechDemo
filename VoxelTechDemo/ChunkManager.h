#pragma once

#include "Chunk.h"
#include <glm/gtx/hash.hpp>
#include <future>
#include <chrono>

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

	std::unordered_map<glm::vec3, Chunk, std::hash<glm::vec3>> m_Chunks;

private:
	//void loadChunk(const glm::vec3& worldPos);
	Chunk loadChunk(glm::vec3&& worldPos);
	
	void unloadChunk(const glm::vec3& worldPos);
	int roundPosToNearestChunk(const float& pos, const int& dim) const;
	void updateVisibleVertices(Chunk& chunk);

	// Not const because I want to eventually use ImGUI to adjust this during runtime
	// 16 is too high for my machine (33 x 33 grid of chunk stacks)
	int m_ChunkRenderRadius = 8;

	static const int NUM_CHUNKS_PER_STACK = 256 / Chunk::Y_DIMENSION;

	std::vector<Chunk> loadChunkStack(const int& x, const int& z);

	int m_LastRoundedPlayerPosX = 0;
	int m_LastRoundedPlayerPosZ = 0;
};