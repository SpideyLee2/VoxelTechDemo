#include "ChunkManager.h"

// This only works if the looping is being done like: for(z){ for(y){ for(x){ }}}
#define position(x,y,z) (x + (y * Chunk2::X_DIMENSION) + (z * Chunk2::Y_DIMENSION * Chunk2::X_DIMENSION))

ChunkManager::ChunkManager(const glm::vec3& playerPos) {
	// Lazy instantiation not necessary as ChunkManager will only need to be instantiated once
	noise = std::make_unique<FastNoiseLite>();
	noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise->SetFrequency(0.01f);
	//noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
	//noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
	//noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
	//noise.SetDomainWarpAmp(50.0f);
	//noise.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
	//noise.SetFractalOctaves(5);
	//noise.SetFractalLacunarity(2.0f);
	//noise.SetFractalGain(0.60f);
	
	const int NUM_CHUNKS = (m_ChunkRenderRadius * 2 + 1) * (m_ChunkRenderRadius * 2 + 1) * NUM_CHUNKS_PER_STACK;

	// Reserve memory enough for every chunk that will be initially rendered
	//m_Chunks.reserve(NUM_CHUNKS);
	
	int m_LastRoundedPlayerPosX = roundPosToNearestChunk(playerPos.x, Chunk2::X_DIMENSION);
	int m_LastRoundedPlayerPosZ = roundPosToNearestChunk(playerPos.z, Chunk2::Z_DIMENSION);

	// Loads the initial chunks around the player
	for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
		// -1 to render radius to adjust for chunks being generated from bottom-left corner (x-z plane)
		for (int z = -m_ChunkRenderRadius; z <= m_ChunkRenderRadius; ++z) {
			for (int x = -m_ChunkRenderRadius; x <= m_ChunkRenderRadius; ++x) {
				Chunk2 chunk = loadChunk(glm::vec3(
					m_LastRoundedPlayerPosX + (Chunk2::X_DIMENSION * x),
					Chunk2::Y_DIMENSION * y,
					m_LastRoundedPlayerPosZ + (Chunk2::Z_DIMENSION * z))
				);
				chunk.generateVAOandVBO();
				chunk.populateVBO();
				//m_Chunks.try_emplace(chunk.m_WorldPos, std::move(chunk));
				m_Chunks.emplace(chunk.m_WorldPos, std::move(chunk));

				//std::cout << "Rendering chunk at: " << chunkWorldPos.x << ", " << chunkWorldPos.y << ", " << chunkWorldPos.z << std::endl;
			}
		}
	}
}

// Rounds the player's position to the nearest possible chunk (x, z) coord for determining
// what chunks to load/unload around the player
int ChunkManager::roundPosToNearestChunk(const float& pos, const int& dim) const {
	return static_cast<int>(std::round(pos / dim)) * dim;
}

// Handles the terrain generation for a chunk
Chunk2 ChunkManager::loadChunk(glm::vec3&& worldPos) {
	//std::cout << "Loading chunk at (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")\n";
	Chunk2 chunk{ worldPos };

	// Puts blocks in the chunk
	for (int z = 0; z < Chunk2::Z_DIMENSION; ++z) {
		for (int x = 0; x < Chunk2::X_DIMENSION; ++x) {
			// Gets the y position of the sampled noise
			int yNoisePos = static_cast<int>(noise->GetNoise(
				x + chunk.m_WorldPos.x,
				z + chunk.m_WorldPos.z
			) * Chunk2::CUBE_OFFSET) + Chunk2::Y_DIMENSION / 2 - 1;
			for (int y = 0; y < Chunk2::Y_DIMENSION; ++y) {
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
	return std::move(chunk);
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
void ChunkManager::updateVisibleVertices(Chunk2& chunk) {
	//bool backVis = false, frontVis = false, leftVis = false, rightVis = false, bottomVis = false, topVis = false;
	chunk.m_NumVisFaces = 0;
	for (int z = 0; z < Chunk2::Z_DIMENSION; ++z) {
		for (int y = 0; y < Chunk2::Y_DIMENSION; ++y) {
			for (int x = 0; x < Chunk2::X_DIMENSION; ++x) {
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
					if ((x < Chunk2::X_DIMENSION - 1 && !chunk.m_pBlocks[position((x + 1), y, z)].m_IsSolid)
						|| x == Chunk2::X_DIMENSION - 1) {
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
					if ((y < Chunk2::Y_DIMENSION - 1 && !chunk.m_pBlocks[position(x, (y + 1), z)].m_IsSolid)
						|| y == Chunk2::Y_DIMENSION - 1) {
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
					if ((z < Chunk2::Z_DIMENSION - 1 && !chunk.m_pBlocks[position(x, y, (z + 1))].m_IsSolid)
						|| z == Chunk2::Z_DIMENSION - 1) {
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

/*
void pushChunk(std::vector<Chunk>& chunks, const int& x, const int& y) {
	//std::cout << "Vector capacity: " << chunks.capacity() << std::endl;
	Chunk chunk{ glm::ivec2(16 * x, 16 * y), camera.m_Position };
	std::lock_guard<std::mutex> lock(chunksMutex);
	chunks.emplace_back(std::move(chunk));
}

void loadChunks(std::vector<Chunk>& chunks, const int renderDistance) {
	for (int y = -renderDistance/2; y < renderDistance/2; ++y) {
		for (int x = -renderDistance/2; x < renderDistance/2; ++x) {
			//pushChunk(chunks, x, y);
			futures.push_back(std::async(std::launch::async, pushChunk, std::ref(chunks), x, y));
		}
	}

	for (std::future<void>& future : futures) {
		future.get();
	}
}
*/

// Determines whether to load/unload chunks and which ones to load/unload
// Adjusts for if the game lags and the player moves across multiple chunks in one frame
void ChunkManager::recalculate(const glm::vec3& playerPos) {
	int currRoundedPlayerPosX = roundPosToNearestChunk(playerPos.x, Chunk2::X_DIMENSION);
	int currRoundedPlayerPosZ = roundPosToNearestChunk(playerPos.z, Chunk2::Z_DIMENSION);

	// Ex: m_ChunkRenderRadius = 1 (3x3 grid of chunk stacks)
	// Player moves from chunk at (0,0,0) to chunk at (32,0,0). Equivalent to a movement of (1,0,0) in the "chunk grid".
	// Unload chunks at (-32,0,32), (-32,0,0), (-32,0,-32)
	// Then, load chunks at (64,0,64), (64,0,32), (64,0,0)

	// If render radius = 1, any translation to another chunk requires loading 3 new chunks and loading 3 previous ones
	// If render radius = 2, load 5, unload 5
	// If render radius = 3, load 7, unload 7
	if (currRoundedPlayerPosX != m_LastRoundedPlayerPosX || currRoundedPlayerPosZ != m_LastRoundedPlayerPosZ) {
		ChunkManager cmInst;
		std::vector<std::future<Chunk2>> futures;

		// Stores the direction (+/-) and number of chunks moved across.
		int xDiff = (currRoundedPlayerPosX - m_LastRoundedPlayerPosX) / Chunk2::X_DIMENSION;
		int zDiff = (currRoundedPlayerPosZ - m_LastRoundedPlayerPosZ) / Chunk2::Z_DIMENSION;
		int xDiffAbs = abs(xDiff);
		int zDiffAbs = abs(zDiff);

		// Reserves enough space to store the future of each chunk that gets loaded
		const int CHUNKS_PER_ROW = (m_ChunkRenderRadius * 2 + 1) * NUM_CHUNKS_PER_STACK;
		futures.reserve(CHUNKS_PER_ROW * xDiffAbs + CHUNKS_PER_ROW * zDiffAbs);

		// Player moved to a new chunk stack in the +/- x direction
		if (xDiff != 0) {
			std::cout << "Player moved from chunk stack at (" 
				<< m_LastRoundedPlayerPosX << ", " << m_LastRoundedPlayerPosZ 
				<< ") to chunk stack at (" 
				<< currRoundedPlayerPosX << ", " << currRoundedPlayerPosZ << ")\n";

			// Unload old chunks that are no longer within the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				// TODO: Batch chunk stack unload calls and do concurrently using std::async
				for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
					unloadChunk(glm::vec3(m_LastRoundedPlayerPosX - xDiff * m_ChunkRenderRadius * Chunk2::X_DIMENSION,
										  y * Chunk2::Y_DIMENSION,
										  m_LastRoundedPlayerPosZ - i * Chunk2::Z_DIMENSION));
				}
			}

			// Load new chunks that entered the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
					futures.emplace_back(std::move(
						// Unavoidable vec3 copy
						std::async(std::launch::async,
								   &ChunkManager::loadChunk,
								   &cmInst,
								   glm::vec3(currRoundedPlayerPosX + xDiff * m_ChunkRenderRadius * Chunk2::X_DIMENSION,
											 y * Chunk2::Y_DIMENSION,
											 currRoundedPlayerPosZ + i * Chunk2::Z_DIMENSION)
						)
					));
				}
			}
		}

		// Player moved to a new chunk stack in the +/- z direction
		if (zDiff != 0) {
			std::cout << "Player moved from chunk stack at ("
				<< m_LastRoundedPlayerPosX << ", " << m_LastRoundedPlayerPosZ
				<< ") to chunk stack at ("
				<< currRoundedPlayerPosX << ", " << currRoundedPlayerPosZ << ")\n";

			// Unload old chunks that are no longer within the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
					unloadChunk(glm::vec3(m_LastRoundedPlayerPosX - i * Chunk2::X_DIMENSION,
										  y * Chunk2::Y_DIMENSION,
										  m_LastRoundedPlayerPosZ - zDiff * m_ChunkRenderRadius * Chunk2::Z_DIMENSION));
				}
			}

			// Load new chunks that entered the render distance
			for (int i = -m_ChunkRenderRadius; i <= m_ChunkRenderRadius; ++i) {
				for (int y = -NUM_CHUNKS_PER_STACK / 2; y < NUM_CHUNKS_PER_STACK / 2; ++y) {
					futures.emplace_back(std::move(
						// Unavoidable vec3 copy
						std::async(std::launch::async,
								   &ChunkManager::loadChunk,
								   &cmInst,
								   glm::vec3(currRoundedPlayerPosX + i * Chunk2::X_DIMENSION,
											 y * Chunk2::Y_DIMENSION,
											 currRoundedPlayerPosZ + zDiff * m_ChunkRenderRadius * Chunk2::Z_DIMENSION)
						)
					));
				}
			}
		}

		// Resynchronize so last rounded player x and z positons can be set
		for (auto& f : futures) {
			Chunk2 chunk = f.get();
			chunk.generateVAOandVBO();
			chunk.populateVBO();
			//m_Chunks.try_emplace(chunk.m_WorldPos, std::move(chunk));
			m_Chunks.emplace(chunk.m_WorldPos, std::move(chunk));
		}
		m_LastRoundedPlayerPosX = currRoundedPlayerPosX;
		m_LastRoundedPlayerPosZ = currRoundedPlayerPosZ;
	}
}