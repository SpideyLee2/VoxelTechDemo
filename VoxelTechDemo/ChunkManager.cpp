#include "ChunkManager.h"

// This only works if the looping is being done like: for(z){ for(y){ for(x){ }}}
#define position(x,y,z) (x + (y * Chunk::X_DIMENSION) + (z * Chunk::Y_DIMENSION * Chunk::X_DIMENSION))

ChunkManager::ChunkManager(const glm::vec3& playerPos) {
	// Lazy instantiation not necessary as ChunkManager will only need to be instantiated once
	noise = std::make_unique<FastNoiseLite>();
	noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise->SetFrequency(0.01f);
	//noise->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
	//noise->SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
	//noise->SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
	//noise->SetDomainWarpAmp(50.0f);
	//noise->SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
	//noise->SetFractalOctaves(5);
	//noise->SetFractalLacunarity(2.0f);
	//noise->SetFractalGain(0.60f);
	
	ChunkManager cmInst;
	std::vector<std::future<Chunk>> futures;

	const int NUM_CHUNKS = (m_ChunkRenderRadius * 2 + 1) * (m_ChunkRenderRadius * 2 + 1) * NUM_CHUNKS_PER_STACK;

	// Reserve memory enough for every chunk that will be initially rendered
	m_Chunks.reserve(NUM_CHUNKS);
	futures.reserve(NUM_CHUNKS);
	
	int m_LastRoundedPlayerPosX = roundPosToNearestChunk(playerPos.x, Chunk::X_DIMENSION);
	int m_LastRoundedPlayerPosZ = roundPosToNearestChunk(playerPos.z, Chunk::Z_DIMENSION);

	// Loads the initial chunks around the player
	for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
		// -1 to render radius to adjust for chunks being generated from bottom-left corner (x-z plane)
		for (int z = -m_ChunkRenderRadius; z <= m_ChunkRenderRadius; ++z) {
			for (int x = -m_ChunkRenderRadius; x <= m_ChunkRenderRadius; ++x) {
				futures.emplace_back(std::move(
					// Unavoidable vec3 copy
					std::async(std::launch::async,
							   &ChunkManager::loadChunk,
							   &cmInst,
							   glm::vec3(
								   m_LastRoundedPlayerPosX + (Chunk::X_DIMENSION * x),
								   Chunk::Y_DIMENSION * y,
								   m_LastRoundedPlayerPosZ + (Chunk::Z_DIMENSION * z)
							   )
					)
				));
			}
		}
	}

	for (auto& f : futures) {
		Chunk chunk = f.get();
		chunk.generateVAOandVBO();
		chunk.populateVBO();
		m_Chunks.try_emplace(chunk.m_WorldPos, std::move(chunk));
		//m_Chunks.emplace(chunk.m_WorldPos, std::move(chunk));
	}
}

// Rounds the player's position to the nearest possible chunk (x, z) coord for determining
// what chunks to load/unload around the player
int ChunkManager::roundPosToNearestChunk(const float& pos, const int& dim) const {
	return static_cast<int>(std::round(pos / dim)) * dim;
}

// Handles the terrain generation for a chunk
Chunk ChunkManager::loadChunk(glm::vec3&& worldPos) {
	//std::cout << "Loading chunk at (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")\n";
	Chunk chunk{ worldPos };

	// Puts blocks in the chunk
	for (int z = 0; z < Chunk::Z_DIMENSION; ++z) {
		for (int x = 0; x < Chunk::X_DIMENSION; ++x) {
			// Gets the y position of the sampled noise
			int yNoisePos = static_cast<int>(noise->GetNoise(
				x + chunk.m_WorldPos.x,
				z + chunk.m_WorldPos.z
			) * Chunk::CUBE_OFFSET) + Chunk::Y_DIMENSION / 2 - 1;
			for (int y = 0; y < Chunk::Y_DIMENSION; ++y) {
				glm::vec3 blockPosInChunk = glm::ivec3(x, y, z);
				glm::mat4 blockModel = glm::translate(glm::mat4(1.0f),
													  blockPosInChunk + chunk.m_WorldPos);
				int yBlockWorldPos = static_cast<int>(std::round(chunk.m_WorldPos.y + y));

				// Determines what block to insert based on the noise pos
				if (yBlockWorldPos == yNoisePos) {
					//std::cout << "Inserting GRASS block at (" << x << ", " << yPos << ", " << z << ")\n";
					chunk.m_pBlocks[position(x, y, z)] =
						std::move(Block(
							BLOCK_TYPE::Grass,
							TEXTURE_TYPE::GrassTop,
							blockModel,
							chunk.getID(),
							blockPosInChunk));
				}
				else if (yBlockWorldPos < yNoisePos) {
					if (yBlockWorldPos < yNoisePos - 5) {
						//std::cout << "Inserting STONE block at (" << x << ", " << y << ", " << z << ")\n";
						chunk.m_pBlocks[position(x, y, z)] = std::move(
							Block(BLOCK_TYPE::Stone,
								  TEXTURE_TYPE::Stone,
								  blockModel,
								  chunk.getID(),
								  blockPosInChunk));
					}
					else {
						//std::cout << "Inserting DIRT block at (" << x << ", " << y << ", " << z << ")\n";
						chunk.m_pBlocks[position(x, y, z)] = std::move(
							Block(BLOCK_TYPE::Dirt,
								  TEXTURE_TYPE::Dirt,
								  blockModel,
								  chunk.getID(),
								  blockPosInChunk));
					}
				}
				else {
					//std::cout << "Inserting AIR block at (" << x << ", " << y << ", " << z << ")\n";
					chunk.m_pBlocks[position(x, y, z)] = std::move(
						Block(BLOCK_TYPE::Air,
							  TEXTURE_TYPE::Water, // Placeholder texture
							  blockModel,
							  chunk.getID(),
							  blockPosInChunk));
				}
			}
		}
	}

	updateVisibleVertices(chunk);
	return chunk;
}

// Removes the chunk at worldPos from the map
void ChunkManager::unloadChunk(const glm::vec3& worldPos) {
	//std::cout << "Unloading chunk at (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")\n";
	// This should be the only instance of this specific chunk
	m_Chunks.erase(worldPos);
}

// Updates the visible vertices vector with the vertices of the faces not occluded by other solid blocks.
// Returns the count of these visible faces.
// WITHOUT away-face culling
void ChunkManager::updateVisibleVertices(Chunk& chunk) {
	//bool backVis = false, frontVis = false, leftVis = false, rightVis = false, bottomVis = false, topVis = false;
	chunk.m_NumVisFaces = 0;
	for (int z = 0; z < Chunk::Z_DIMENSION; ++z) {
		for (int y = 0; y < Chunk::Y_DIMENSION; ++y) {
			for (int x = 0; x < Chunk::X_DIMENSION; ++x) {
				//updateVFM(x, y, z);
				Block& block = chunk.m_pBlocks[position(x, y, z)];
				if (block.m_IsSolid) {
					int texType = static_cast<int>(block.getTexType());
					// Appends left face if true
					if (((x > 0 && !chunk.m_pBlocks[position((x - 1), y, z)].m_IsSolid) || x == 0)) {
						//std::cout << "Appending left face\n";
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);

						++chunk.m_NumVisFaces;
					}
					// Appends right face if true
					if ((x < Chunk::X_DIMENSION - 1 && !chunk.m_pBlocks[position((x + 1), y, z)].m_IsSolid)
						|| x == Chunk::X_DIMENSION - 1) {
						//std::cout << "Appending right face\n";
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);

						++chunk.m_NumVisFaces;
					}
					// Appends bottom face if true
					if ((y > 0 && !chunk.m_pBlocks[position(x, (y - 1), z)].m_IsSolid) || y == 0) {
						//std::cout << "Appending bottom face\n";
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);

						++chunk.m_NumVisFaces;
					}
					// Appends top face if true
					if ((y < Chunk::Y_DIMENSION - 1 && !chunk.m_pBlocks[position(x, (y + 1), z)].m_IsSolid)
						|| y == Chunk::Y_DIMENSION - 1) {
						//std::cout << "Appending top face\n";
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);

						++chunk.m_NumVisFaces;
					}
					// Appends back face if true
					if ((z > 0 && !chunk.m_pBlocks[position(x, y, (z - 1))].m_IsSolid) || z == 0) {
						//std::cout << "Appending back face\n";
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);

						++chunk.m_NumVisFaces;
					}
					// Append front face if true
					if ((z < Chunk::Z_DIMENSION - 1 && !chunk.m_pBlocks[position(x, y, (z + 1))].m_IsSolid)
						|| z == Chunk::Z_DIMENSION - 1) {
						//std::cout << "Appending front face\n";
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// bottom-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{ glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
							glm::vec3(0.0f,  0.0f, 1.0f),
							glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// top-right
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// top-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						chunk.m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);

						++chunk.m_NumVisFaces;
					}
				}
			}
		}
	}
}

void ChunkManager::renderChunks(const Shader& shader) {
	for (const auto& chunkPair : m_Chunks) {
		chunkPair.second.render(shader);
	}
}

// (DEBUGGING) Renders a red cube at the position that the bottom-center of every chunk should be
void ChunkManager::renderChunkCenters(const Shader& shader, const unsigned int& vao) {
	glBindVertexArray(vao);
	for (const auto& chunkPair : m_Chunks) {
		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, glm::value_ptr(chunkPair.second.m_Model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

// Determines whether to load/unload chunks and which ones to load/unload
// Adjusts for if the game lags and the player moves across multiple chunks in one frame
void ChunkManager::recalculate(const glm::vec3& playerPos) {
	int currRoundedPlayerPosX = roundPosToNearestChunk(playerPos.x, Chunk::X_DIMENSION);
	int currRoundedPlayerPosZ = roundPosToNearestChunk(playerPos.z, Chunk::Z_DIMENSION);

	// If the player moved to a new chunk this frame, loads and unloads necessary chunks to maintain render radius
	if (currRoundedPlayerPosX != m_LastRoundedPlayerPosX || currRoundedPlayerPosZ != m_LastRoundedPlayerPosZ) {
		std::cout << "Player moved from chunk stack at ("
			<< m_LastRoundedPlayerPosX << ", " << m_LastRoundedPlayerPosZ
			<< ") to chunk stack at ("
			<< currRoundedPlayerPosX << ", " << currRoundedPlayerPosZ << ")\n";

		auto startChunkGen = std::chrono::steady_clock::now();
		ChunkManager cmInst;

		// Stores the direction (+/-) and number of chunks moved across.
		int xDiff = (currRoundedPlayerPosX - m_LastRoundedPlayerPosX) / Chunk::X_DIMENSION;
		int zDiff = (currRoundedPlayerPosZ - m_LastRoundedPlayerPosZ) / Chunk::Z_DIMENSION;

		// Reserves enough space to store the future of each chunk that gets loaded
		const int CHUNK_STACKS_PER_ROW = m_ChunkRenderRadius * 2 + 1;
		const int CHUNK_STACKS_TO_LOAD = CHUNK_STACKS_PER_ROW * abs(xDiff) + CHUNK_STACKS_PER_ROW * abs(zDiff);
		std::vector<std::future<std::vector<Chunk>>> futures;
		futures.reserve(CHUNK_STACKS_TO_LOAD);

		std::chrono::duration<double, std::milli> duration;
		// Player moved to a new chunk stack in the +/- x direction
		if (xDiff != 0) {
			// Unload old chunks that are no longer within the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				// TODO: Profile batching concurrent chunk stack unload calls with std::async
				for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
					unloadChunk(glm::vec3(m_LastRoundedPlayerPosX - xDiff * m_ChunkRenderRadius * Chunk::X_DIMENSION,
										  y * Chunk::Y_DIMENSION,
										  m_LastRoundedPlayerPosZ - i * Chunk::Z_DIMENSION)
					);
				}
			}

			// Load new chunks that entered the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				// Profiled. Loading chunk stacks concurrently is faster than loading individual chunks concurrently
				futures.emplace_back(std::move(
					std::async(std::launch::async,
								&ChunkManager::loadChunkStack,
								&cmInst,
								currRoundedPlayerPosX + xDiff * m_ChunkRenderRadius * Chunk::X_DIMENSION,
								currRoundedPlayerPosZ + i * Chunk::Z_DIMENSION
					)
				));
			}
		}

		// Player moved to a new chunk stack in the +/- z direction
		if (zDiff != 0) {
			// Unload old chunks that are no longer within the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
					// TODO: Profile batching concurrent chunk stack unload calls with std::async
					unloadChunk(glm::vec3(m_LastRoundedPlayerPosX - i * Chunk::X_DIMENSION,
										  y * Chunk::Y_DIMENSION,
										  m_LastRoundedPlayerPosZ - zDiff * m_ChunkRenderRadius * Chunk::Z_DIMENSION)
					);
				}
			}

			// Load new chunks that entered the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				// Profiled. Loading chunk stacks concurrently is faster than loading individual chunks concurrently
				futures.emplace_back(std::move(
					std::async(std::launch::async,
							   &ChunkManager::loadChunkStack,
							   &cmInst,
							   currRoundedPlayerPosX + i * Chunk::X_DIMENSION,
							   currRoundedPlayerPosZ + zDiff * m_ChunkRenderRadius * Chunk::Z_DIMENSION
					)
				));
			}
		}

		auto startFutures = std::chrono::steady_clock::now();
		// Resynchronize
		for (auto& f : futures) {
			auto chunks = f.get();
			for (Chunk& chunk : chunks) {
				chunk.generateVAOandVBO();
				chunk.populateVBO();

				m_Chunks.try_emplace(chunk.m_WorldPos, std::move(chunk));
				//m_Chunks.emplace(chunk.m_WorldPos, std::move(chunk));
			}
		}
		auto endFutures = std::chrono::steady_clock::now();

		m_LastRoundedPlayerPosX = currRoundedPlayerPosX;
		m_LastRoundedPlayerPosZ = currRoundedPlayerPosZ;

		auto endChunkGen = std::chrono::steady_clock::now();

		// Elapsed times
		const int TOTAL_CHUNKS_TO_LOAD = NUM_CHUNKS_PER_STACK * CHUNK_STACKS_TO_LOAD;
		duration = endFutures - startFutures;
		std::cout << "Time to handle futures of " << TOTAL_CHUNKS_TO_LOAD << " chunks: "
			<< duration.count() << "ms    AVG: " 
			<< duration.count() / TOTAL_CHUNKS_TO_LOAD << "ms\n";

		duration = endChunkGen - startChunkGen;
		std::cout << "====== Time to load/unload " << TOTAL_CHUNKS_TO_LOAD << " chunks: "
			<< duration.count() << "ms    AVG: " 
			<< duration.count() / TOTAL_CHUNKS_TO_LOAD << "ms ======\n\n";
	}
}

// Loads a stack of chunks at (x, z)
std::vector<Chunk> ChunkManager::loadChunkStack(const int& x, const int& z) {
	std::vector<Chunk> chunks;
	chunks.reserve(NUM_CHUNKS_PER_STACK);
	for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
		chunks.emplace_back(loadChunk(glm::vec3(x, y * Chunk::Y_DIMENSION, z)));
	}

	return chunks;
}

// Populates the adjacent blocks member of each block for occlusion culling purposes
/*
for (int z = 0; z < Z_DIMENSION; ++z) {
	for (int x = 0; x < X_DIMENSION; ++x) {
		for (int y = 0; y < Y_DIMENSION; ++y) {
			std::string blockType = m_pBlocks[position(x, y, z)].getBlockType() == BLOCK_TYPE::Air ? "Air" : "Solid";
			std::cout << "Block Type: " << blockType;
			std::cout << "Setting adjacent blocks...\n";
			x == 0 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 2)
				   : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x - 1, y, z)].getBlockType(), 2);
			y == 0 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 4)
				   : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y - 1, z)].getBlockType(), 4);
			z == 0 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 0)
				   : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y, z - 1)].getBlockType(), 0);
			x == X_DIMENSION - 1 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 3)
								 : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x + 1, y, z)].getBlockType(), 3);
			y == Y_DIMENSION - 1 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 5)
								 : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y + 1, z)].getBlockType(), 5);
			z == Z_DIMENSION - 1 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 1)
								 : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y, z + 1)].getBlockType(), 1);
		}
	}
}
*/