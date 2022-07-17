#include "Chunk.h"

#define position(x,y,z) (x + (y * X_DIMENSION) + (z * Y_DIMENSION * X_DIMENSION))

Chunk::Chunk(glm::vec2&& worldPos) : m_ID(++s_ChunkIDCounter), m_WorldPos(worldPos), m_VisVerts(std::vector<Vertex>()) {
	m_pBlocks = std::make_unique<Block[]>(VOLUME);

	if (noise == nullptr) {
		noise = std::make_unique<FastNoiseLite>();
		noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
		noise->SetFrequency(0.010f);
		//noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
		//noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
		//noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
		//noise.SetDomainWarpAmp(50.0f);
		//noise.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
		//noise.SetFractalOctaves(5);
		//noise.SetFractalLacunarity(2.0f);
		//noise.SetFractalGain(0.60f);
	}

	//int noiseInd = 0;
	glm::vec3 worldPosOffset{ m_WorldPos.x, 0.0f, m_WorldPos.y };
	for (int z = 0; z < Z_DIMENSION; ++z) {
		for (int x = 0; x < X_DIMENSION; ++x) {
			// Creates the block at a position based on noise
			int yPos = (int)(noise->GetNoise((float)x + m_WorldPos.x, (float)z + m_WorldPos.y) * CUBE_OFFSET) + 127;
			glm::vec3 chunkPos = glm::ivec3(x, yPos, z);
			glm::mat4 model = glm::translate(glm::mat4(1.0f), chunkPos + worldPosOffset);
			//m_pModelMatrices[position(x, yPos, z)] = model;
			//std::cout << "Inserting GRASS block at (" << x << ", " << yPos << ", " << z << ")\n";
			m_pBlocks[position(x, yPos, z)] =
				std::move(Block(
					BLOCK_TYPE::Grass,
					TEXTURE_TYPE::GrassTop,
					model,
					m_ID,
					chunkPos));

			// Fills in all the cubes above with air blocks
			for (int y = yPos + 1; y < Y_DIMENSION; ++y) {
				chunkPos = glm::ivec3(x, y, z);
				model = glm::translate(glm::mat4(1.0f), chunkPos + worldPosOffset);
				//m_pModelMatrices[position(x, y, z)] = model;
				//std::cout << "Inserting AIR block at (" << x << ", " << y << ", " << z << ")\n";
				m_pBlocks[position(x, y, z)] = std::move(
					Block(BLOCK_TYPE::Air,
						  TEXTURE_TYPE::Water,
						  model,
						  m_ID,
						  chunkPos));
			}

			// Fills in all the cubes below with dirt blocks
			for (int y = yPos - 1; y >= 0; --y) {
				chunkPos = glm::ivec3(x, y, z);
				model = glm::translate(glm::mat4(1.0f), chunkPos + worldPosOffset);
				//m_pModelMatrices[position(x, y, z)] = model;
				if (y + 5 < yPos) {
					//std::cout << "Inserting STONE block at (" << x << ", " << y << ", " << z << ")\n";
					m_pBlocks[position(x, y, z)] = std::move(
						Block(BLOCK_TYPE::Stone,
							  TEXTURE_TYPE::Stone,
							  model,
							  m_ID,
							  chunkPos));
				}
				else {
					//std::cout << "Inserting DIRT block at (" << x << ", " << y << ", " << z << ")\n";
					m_pBlocks[position(x, y, z)] = std::move(
						Block(BLOCK_TYPE::Dirt,
							  TEXTURE_TYPE::Dirt,
							  model,
							  m_ID,
							  chunkPos));
				}
			}
		}
	}
	updateVisibleFacesMesh();
	//populateVBO();

	// Populates the adjacent blocks member of each block for occlusion culling purposes
	//for (int z = 0; z < Z_DIMENSION; ++z) {
	//	for (int x = 0; x < X_DIMENSION; ++x) {
	//		for (int y = 0; y < Y_DIMENSION; ++y) {
	//			std::string blockType = m_pBlocks[position(x, y, z)].getBlockType() == BLOCK_TYPE::Air ? "Air" : "Solid";
	//			std::cout << "Block Type: " << blockType;
	//			std::cout << "Setting adjacent blocks...\n";
	//			x == 0 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 2)
	//				   : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x - 1, y, z)].getBlockType(), 2);
	//			y == 0 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 4)
	//				   : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y - 1, z)].getBlockType(), 4);
	//			z == 0 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 0)
	//				   : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y, z - 1)].getBlockType(), 0);
	//			x == X_DIMENSION - 1 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 3)
	//								 : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x + 1, y, z)].getBlockType(), 3);
	//			y == Y_DIMENSION - 1 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 5)
	//								 : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y + 1, z)].getBlockType(), 5);
	//			z == Z_DIMENSION - 1 ? m_pBlocks[position(x, y, z)].setAdjBlockType(BLOCK_TYPE::Air, 1)
	//								 : m_pBlocks[position(x, y, z)].setAdjBlockType(m_pBlocks[position(x, y, z + 1)].getBlockType(), 1);
	//		}
	//	}
	//}
}

void Chunk::generateVAOandVBO() {
	glGenVertexArrays(1, &m_Vao);
	glGenBuffers(1, &m_Vbo);
}

void Chunk::populateVBO() {
	glBindVertexArray(m_Vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
	glBufferData(GL_ARRAY_BUFFER, m_VisVerts.size() * sizeof(Vertex), m_VisVerts.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Chunk::render(const Shader& shader, const bool& cameraHasMoved) {
	if (cameraHasMoved) {
		updateVisibleFacesMesh();
		populateVBO();
	}
	shader.use();

	//Block* block;
	//for (int x = 0; x < X_DIMENSION; ++x) {
	//	for (int y = 0; y < Y_DIMENSION; ++y) {
	//		for (int z = 0; z < Z_DIMENSION; ++z) {
	//			block = &m_pBlocks[position(x, y, z)];
	//			if (block == nullptr) {
	//				std::cout << "This block position hasn't been assigned a block yet (BUG)\n";
	//			}
	//			else if (block->getBlockType() != BLOCK_TYPE::Air) {
	//				glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, glm::value_ptr(block->m_Model));
	//				glDrawArrays(GL_TRIANGLES, 0, 36);
	//			}
	//			//else {
	//			//	std::cout << "block (" << x << ", " << y << ", " << z << ") was either never initialized or is an air block" << std::endl;
	//			//}
	//		}
	//	}
	//}
	//block = nullptr;
	//delete block;

	glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, 
					   glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(m_WorldPos.x, 0.0f, m_WorldPos.y))));

	glBindVertexArray(m_Vao);
	glDrawArrays(GL_TRIANGLES, 0, m_NumVisFaces * Block::VERTS_PER_FACE);
}

// This should be supplied as a callable to std::async
void Chunk::updateVFM(const int& x, const int& y, const int& z) {

}

// Updates the visible vertices vector with the vertices of the faces not occluded by other solid blocks.
// Returns the count of these visible faces.
void Chunk::updateVisibleFacesMesh() {
	//bool backVis = false, frontVis = false, leftVis = false, rightVis = false, bottomVis = false, topVis = false;
	m_NumVisFaces = 0;
	for (int z = 0; z < Z_DIMENSION; ++z) {
		for (int y = 0; y < Y_DIMENSION; ++y) {
			for (int x = 0; x < X_DIMENSION; ++x) {
				//updateVFM(x, y, z);
				Block* block = &m_pBlocks[position(x, y, z)];
				if (block->m_IsSolid) {
					int texType = static_cast<int>(block->getTexType());
					// Appends left face if true
					if ((x > 0 && !m_pBlocks[position((x - 1), y, z)].m_IsSolid) || x == 0) {
						//std::cout << "Appending left face\n";
						//leftVis = true;
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(-1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);

						++m_NumVisFaces;
					}
					// Appends right face if true
					if ((x < X_DIMENSION - 1 && !m_pBlocks[position((x + 1), y, z)].m_IsSolid) || x == X_DIMENSION - 1) {
						//std::cout << "Appending right face\n";
						//rightVis = true;
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);

						++m_NumVisFaces;
					}
					// Appends bottom face if true
					if ((y > 0 && !m_pBlocks[position(x, (y - 1), z)].m_IsSolid) || y == 0) {
						//std::cout << "Appending bottom face\n";
						//bottomVis = true;
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);

						++m_NumVisFaces;
					}
					// Appends top face if true
					if ((y < Y_DIMENSION - 1 && !m_pBlocks[position(x, (y + 1), z)].m_IsSolid) || y == Y_DIMENSION - 1) {
						//std::cout << "Appending top face\n";
						//topVis = true;
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  1.0f,  0.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);

						++m_NumVisFaces;
					}
					// Appends back face if true
					if ((z > 0 && !m_pBlocks[position(x, y, (z - 1))].m_IsSolid) || z == 0) {
						//std::cout << "Appending back face\n";
						//backVis = true;
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
								glm::vec3(0.0f,  0.0f, -1.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);

						++m_NumVisFaces;
					}
					// Append front face if true
					if ((z < Z_DIMENSION - 1 && !m_pBlocks[position(x, y, (z + 1))].m_IsSolid) || z == Z_DIMENSION - 1) {
						//std::cout << "Appending front face\n";
						//frontVis = true;
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);
						// bottom-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(1.0f, 0.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{ glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
							glm::vec3(0.0f,  0.0f, 1.0f),
							glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// top-right
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(1.0f, 1.0f, texType)
							}
						);
						// top-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						);
						// bottom-left
						m_VisVerts.push_back(
							Vertex{
								glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
								glm::vec3(0.0f,  0.0f, 1.0f),
								glm::vec3(0.0f, 0.0f, texType)
							}
						);

						++m_NumVisFaces;
					}
				}
			}
		}
	}
	//for (std::future<void>& future : futures) {
	//	future.get();
	//}
}

Block& Chunk::getBlockAt(const glm::ivec3& blockPosInChunk) const {
	int x = blockPosInChunk.x;
	int y = blockPosInChunk.y;
	int z = blockPosInChunk.z;
	return m_pBlocks[position(x,y,z)];
}

Chunk::Chunk(Chunk&& chunk) noexcept : 
	m_ID(std::move(chunk.m_ID)),
	m_WorldPos(std::move(chunk.m_WorldPos)), 
	m_VisVerts(std::move(chunk.m_VisVerts)), 
	m_pBlocks(std::move(chunk.m_pBlocks)),
	m_Vao(std::move(chunk.m_Vao)),
	m_Vbo(std::move(chunk.m_Vbo)),
	m_NumVisFaces(std::move(chunk.m_NumVisFaces)) {

}

Chunk& Chunk::operator=(Chunk&& chunk) noexcept {
	if (this != &chunk) {
		this->m_Vao = std::move(chunk.m_Vao);
		this->m_Vbo = std::move(chunk.m_Vbo);
		this->m_WorldPos = std::move(chunk.m_WorldPos);
		this->m_pBlocks = std::move(chunk.m_pBlocks);
		this->m_ID = std::move(chunk.m_ID);
		this->m_NumVisFaces = std::move(chunk.m_NumVisFaces);
		this->m_VisVerts = std::move(chunk.m_VisVerts);
	}

	return *this;
}