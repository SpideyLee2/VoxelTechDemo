#include "ChunkManager.h"

// This only works if the looping is being done like: for(z){ for(y){ for(x){ }}}
#define position(x,y,z) (x + (y * Chunk2::X_DIMENSION) + (z * Chunk2::Y_DIMENSION * Chunk2::X_DIMENSION))

ChunkManager::ChunkManager(const glm::vec3& playerPos) {
	if (noise == nullptr) {
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
	}
	
	const int NUM_CHUNKS = m_ChunkRenderRadius * m_NumChunksPerStack * m_ChunkRenderRadius;

	// Reserve memory enough for every chunk that will be initially rendered
	m_Chunks.reserve(NUM_CHUNKS);
	
	// Rounds player pos to the nearest chunk for determining which chunks to load
	int roundedPlayerPosX = roundPosToNearestChunk(playerPos.x);
	int roundedPlayerPosZ = roundPosToNearestChunk(playerPos.z);

	// Loads the initial chunks around the player
	for (int y = -m_NumChunksPerStack / 2; y < m_NumChunksPerStack / 2; ++y) {
		for (int z = -m_ChunkRenderRadius; z < m_ChunkRenderRadius; ++z) {
			for (int x = -m_ChunkRenderRadius; x < m_ChunkRenderRadius; ++x) {
				glm::vec3 chunkWorldPos = glm::vec3(
					roundedPlayerPosX + (Chunk2::X_DIMENSION * x),
					Chunk2::Y_DIMENSION * y,
					roundedPlayerPosZ + (Chunk2::Z_DIMENSION * z)
				);
				loadChunk(chunkWorldPos);
				Chunk2& chunk = m_Chunks.at(chunkWorldPos);
				chunk.generateVAOandVBO();
				chunk.populateVBO();
				std::cout << "Rendering chunk at: " << chunkWorldPos.x << ", " << chunkWorldPos.y << ", " << chunkWorldPos.z << std::endl;
			}
		}
	}
}

int ChunkManager::roundPosToNearestChunk(const float& pos) const {
	// This assumes chunk x and z dimensions are the same. 
	// Will otherwise not work properly.
	int mod = static_cast<int>(std::round(pos)) % Chunk2::X_DIMENSION;
	if (mod < Chunk2::X_DIMENSION / 2)
		return pos - mod;
	return pos + Chunk2::X_DIMENSION - mod;

	// EXAMPLE: playerPos = (60.4, 0, 37.12)

	// === For Rounded X Coord ===
	// iPlayerPos = 60
	// mod = 60 % 32 = 28
	// 28 < 16 = false
	// roundedPlayerPos.x = 60 + 32 - 28 = 64

	// === For Rounded Z Coord ===
	// iPlayerPos = 37
	// mod = 37 % 32 = 5
	// 5 < 16 = true
	// roundedPlayerPos.y = 37 - 5 = 32
}

void ChunkManager::loadChunk(const glm::vec3& worldPos) {
	Chunk2 chunk{ worldPos };

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
	updateVisibleFacesMesh(chunk);
	
	m_Chunks.try_emplace(chunk.m_WorldPos, std::move(chunk));
}

// WITHOUT away-face culling
void ChunkManager::updateVisibleFacesMesh(Chunk2& chunk) {
	//bool backVis = false, frontVis = false, leftVis = false, rightVis = false, bottomVis = false, topVis = false;
	chunk.m_NumVisFaces = 0;
	for (int z = 0; z < Chunk2::Z_DIMENSION; ++z) {
		for (int y = 0; y < Chunk2::Y_DIMENSION; ++y) {
			for (int x = 0; x < Chunk2::X_DIMENSION; ++x) {
				//updateVFM(x, y, z);
				Block* block = &chunk.m_pBlocks[position(x, y, z)];
				if (block->m_IsSolid) {
					int texType = static_cast<int>(block->getTexType());
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

void ChunkManager::unloadChunk(Chunk2& chunk) {

}

void ChunkManager::renderChunks(const Shader& shader) {
	for (const auto& chunkPair : m_Chunks) {
		chunkPair.second.render(shader);
	}
}

// Renders a red cube at the position that the bottom-center of every chunk should be
void ChunkManager::renderChunkCenters(const Shader& shader, const unsigned int& vao) {
	glBindVertexArray(vao);
	for (const auto& chunkPair : m_Chunks) {
		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, glm::value_ptr(chunkPair.second.m_Model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}