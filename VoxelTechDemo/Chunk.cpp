#include "Chunk.h"

#define position(x,y,z) (x + (y * X_DIMENSION) + (z * Y_DIMENSION * X_DIMENSION))

Chunk::Chunk(glm::vec2&& worldPos) : m_ID(++s_ChunkIDCounter), m_WorldPos(worldPos), m_VisVerts(std::vector<Vertex>()) {
	m_pBlocks = std::make_unique<Block[]>(VOLUME);
	glGenVertexArrays(1, &m_Vao);
	glGenBuffers(1, &m_Vbo);

	//if (blockTextures == nullptr) {
	//	blockTextures = new std::unordered_map<TEXTURE_TYPE, const std::shared_ptr<Texture2D>>();

	//	blockTextures->insert({ TEXTURE_TYPE::Air, std::make_shared<Texture2D>("textures/Full_Specular.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::GrassTop, std::make_shared<Texture2D>("textures/grass/Block_Grass_Top.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::GrassSide, std::make_shared<Texture2D>("textures/grass/Block_Grass_Side.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::Dirt, std::make_shared<Texture2D>("textures/dirt/Block_Dirt.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::Stone, std::make_shared<Texture2D>("textures/stone/Block_Stone.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::Sand, std::make_shared<Texture2D>("textures/sand/Block_Sand.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::Water, std::make_shared<Texture2D>("textures/water/Block_Water.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::FullSpecular, std::make_shared<Texture2D>("textures/Full_Specular.png") });
	//	blockTextures->insert({ TEXTURE_TYPE::NoSpecular, std::make_shared<Texture2D>("textures/No_Specular.png") });
	//}

	if (noise == nullptr) {
		noise = std::make_unique<FastNoiseLite>();
		noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
		noise->SetFrequency(0.025f);
		//noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
		//noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
		//noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
		//noise.SetDomainWarpAmp(50.0f);
		//noise.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
		//noise.SetFractalOctaves(5);
		//noise.SetFractalLacunarity(2.0f);
		//noise.SetFractalGain(0.60f);
	}

	generate();
}

void Chunk::generate() {
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
						  TEXTURE_TYPE::Air,
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
	numVisFaces = updateVisibleFacesMesh();
	populateVBO();
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

void Chunk::populateVBO() {
	glBindVertexArray(m_Vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
	glBufferData(GL_ARRAY_BUFFER, m_VisVerts.size() * sizeof(Vertex), m_VisVerts.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Block::ELEMS_PER_VERT * sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, Block::ELEMS_PER_VERT * sizeof(Vertex), (void*)offsetof(Vertex, position));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, Block::ELEMS_PER_VERT * sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Chunk::render(const Shader& shader, const bool& cameraHasMoved) {
	if (cameraHasMoved) {
		numVisFaces = updateVisibleFacesMesh();
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

	glDrawArrays(GL_TRIANGLES, 0, numVisFaces * Block::VERTS_PER_FACE);
}

// Updates the m_VisVerts vector with the vertices of the faces not occluded by other solid blocks.
int Chunk::updateVisibleFacesMesh() {
	//bool backVis = false, frontVis = false, leftVis = false, rightVis = false, bottomVis = false, topVis = false;
	int totalVisFaces = 0;
	int numVisFaces = 0;
	for (int z = 0; z < Z_DIMENSION; ++z) {
		for (int y = 0; y < Y_DIMENSION; ++y) {
			for (int x = 0; x < X_DIMENSION; ++x) {
				//std::cout << "Iteration in loop: (" << x << ", " << y << ", " << z << ")\n";
				//std::cout << "Array position of (x,y,z): " << position(x, y, z) << std::endl;
				//std::cout << "Array position of (x,y-1,z): " << position(x, (y - 1), z) << std::endl;
				Block* block = &m_pBlocks[position(x, y, z)];
				if (block->m_IsSolid) {
					int texType = static_cast<int>(block->getTexType());
					if (x > 0 && !m_pBlocks[position((x - 1), y, z)].m_IsSolid) { // Append left face
						std::cout << "Appending left face\n";
						//leftVis = true;
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(-0.5f, -0.5f, -0.5f),	
								glm::vec3(0.0f,  0.0f, -1.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(0.5f,  0.5f, -0.5f),	
								glm::vec3(0.0f,  0.0f, -1.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(0.5f, -0.5f, -0.5f),	
								glm::vec3(0.0f,  0.0f, -1.0f),	
								glm::vec3(0.0f, 0.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(0.5f,  0.5f, -0.5f),	
								glm::vec3(0.0f,  0.0f, -1.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(-0.5f, -0.5f, -0.5f),	
								glm::vec3(0.0f,  0.0f, -1.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(-0.5f,  0.5f, -0.5f),	
								glm::vec3(0.0f,  0.0f, -1.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}); 

						++numVisFaces;
					}
					if (x < X_DIMENSION - 1 && !m_pBlocks[position((x + 1), y, z)].m_IsSolid) { // Append right face
						std::cout << "Appending right face\n";
						//rightVis = true;
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(-0.5f, -0.5f,  0.5f),	
								glm::vec3(0.0f,  0.0f, 1.0f),	
								glm::vec3(0.0f, 0.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(0.5f, -0.5f,  0.5f),	
								glm::vec3(0.0f,  0.0f, 1.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(0.5f,  0.5f,  0.5f),	
								glm::vec3(0.0f,  0.0f, 1.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(0.5f,  0.5f,  0.5f),	
								glm::vec3(0.0f,  0.0f, 1.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(-0.5f,  0.5f,  0.5f),	
								glm::vec3(0.0f,  0.0f, 1.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}); 
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(-0.5f, -0.5f,  0.5f),	
								glm::vec3(0.0f,  0.0f, 1.0f),	
								glm::vec3(0.0f, 0.0f, texType) 
							}); 

						++numVisFaces;
					}
					if (y > 0 && !m_pBlocks[position(x, (y - 1), z)].m_IsSolid) { // Append bottom face
						std::cout << "Appending bottom face\n";
						//bottomVis = true;
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(-0.5f,  0.5f,  0.5f),	
								glm::vec3(-1.0f, 0.0f, 0.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}
						);
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(-0.5f,  0.5f, -0.5f),	
								glm::vec3(-1.0f, 0.0f, 0.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}
						);
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(-0.5f, -0.5f, -0.5f),	
								glm::vec3(-1.0f, 0.0f, 0.0f),	
								glm::vec3(0.0f, 0.0f, texType) 
							}
						);
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(-0.5f, -0.5f, -0.5f),	
								glm::vec3(-1.0f, 0.0f, 0.0f),	
								glm::vec3(0.0f, 0.0f, texType) 
							}
						);
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(-0.5f, -0.5f,  0.5f),	
								glm::vec3(-1.0f, 0.0f, 0.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(-0.5f,  0.5f,  0.5f),	
								glm::vec3(-1.0f, 0.0f, 0.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}
						); 

						++numVisFaces;
					}
					if (y < Y_DIMENSION - 1 && !m_pBlocks[position(x, (y + 1), z)].m_IsSolid) { // Append top face
						std::cout << "Appending top face\n";
						//topVis = true;
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(0.5f,  0.5f,  0.5f),	
								glm::vec3(1.0f, 0.0f, 0.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(0.5f, -0.5f, -0.5f),	
								glm::vec3(1.0f, 0.0f, 0.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(0.5f,  0.5f, -0.5f),	
								glm::vec3(1.0f, 0.0f, 0.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(0.5f, -0.5f, -0.5f),	
								glm::vec3(1.0f, 0.0f, 0.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(0.5f,  0.5f,  0.5f),	
								glm::vec3(1.0f, 0.0f, 0.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(0.5f, -0.5f,  0.5f),	
								glm::vec3(1.0f, 0.0f, 0.0f),	
								glm::vec3(0.0f, 0.0f, texType) 
							}
						); 

						++numVisFaces;
					}
					if (z > 0 && !m_pBlocks[position(x, y, (z - 1))].m_IsSolid) { // Append back face
						std::cout << "Appending back face\n";
						//backVis = true;
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(-0.5f, -0.5f, -0.5f),	
								glm::vec3(0.0f, -1.0f, 0.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(0.5f, -0.5f, -0.5f),	
								glm::vec3(0.0f, -1.0f, 0.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(0.5f, -0.5f,  0.5f),	
								glm::vec3(0.0f, -1.0f, 0.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(0.5f, -0.5f,  0.5f),	
								glm::vec3(0.0f, -1.0f, 0.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(-0.5f, -0.5f,  0.5f),	
								glm::vec3(0.0f, -1.0f, 0.0f),	
								glm::vec3(0.0f, 0.0f, texType)
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(-0.5f, -0.5f, -0.5f),
								glm::vec3(0.0f, -1.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, texType)
							}
						); 

						++numVisFaces;
					}
					if (z < Z_DIMENSION - 1 && !m_pBlocks[position(x, y, (z + 1))].m_IsSolid) { // Append front face
						std::cout << "Appending front face\n";
						//frontVis = true;
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(-0.5f,  0.5f, -0.5f),	
								glm::vec3(0.0f,  1.0f,  0.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(0.5f,  0.5f,  0.5f),	
								glm::vec3(0.0f,  1.0f,  0.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // top-right
								glm::vec3(0.5f,  0.5f, -0.5f),	
								glm::vec3(0.0f,  1.0f,  0.0f),	
								glm::vec3(1.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-right
								glm::vec3(0.5f,  0.5f,  0.5f),	
								glm::vec3(0.0f,  1.0f,  0.0f),	
								glm::vec3(1.0f, 0.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // top-left
								glm::vec3(-0.5f,  0.5f, -0.5f),	
								glm::vec3(0.0f,  1.0f,  0.0f),	
								glm::vec3(0.0f, 1.0f, texType) 
							}
						); 
						m_VisVerts.push_back(
							Vertex{ // bottom-left
								glm::vec3(-0.5f,  0.5f,  0.5f),	
								glm::vec3(0.0f,  1.0f,  0.0f),	
								glm::vec3(0.0f, 0.0f, texType) 
							}
						); 
						
						++numVisFaces;
					}
				}
				totalVisFaces += numVisFaces;
				numVisFaces = 0;
			}
		}
	}
	return totalVisFaces;
}

Block& Chunk::getBlockAt(glm::ivec3 blockPosInChunk) const {
	int x = blockPosInChunk.x;
	int y = blockPosInChunk.y;
	int z = blockPosInChunk.z;
	return m_pBlocks[position(x,y,z)];
}
