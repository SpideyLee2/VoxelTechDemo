#include "Chunk.h"

#define position(x,y,z) (x + (y * X_DIMENSION) + (z * Y_DIMENSION * X_DIMENSION))

Chunk::Chunk(glm::vec2&& worldPos) : m_ID(s_chunkID++), m_worldPos(worldPos) {
	m_pBlocks = std::make_unique<Block[]>(VOLUME);

	if (blockTextures == nullptr) {
		blockTextures = new std::unordered_map<TEXTURE_TYPE, const std::shared_ptr<Texture2D>>();

		blockTextures->insert({ TEXTURE_TYPE::Air, std::make_shared<Texture2D>("textures/Full_Specular.png") });
		blockTextures->insert({ TEXTURE_TYPE::GrassTop, std::make_shared<Texture2D>("textures/grass/Block_Grass_Top.png") });
		blockTextures->insert({ TEXTURE_TYPE::GrassSide, std::make_shared<Texture2D>("textures/grass/Block_Grass_Side.png") });
		blockTextures->insert({ TEXTURE_TYPE::Dirt, std::make_shared<Texture2D>("textures/dirt/Block_Dirt.png") });
		blockTextures->insert({ TEXTURE_TYPE::Stone, std::make_shared<Texture2D>("textures/stone/Block_Stone.png") });
		blockTextures->insert({ TEXTURE_TYPE::Sand, std::make_shared<Texture2D>("textures/sand/Block_Sand.png") });
		blockTextures->insert({ TEXTURE_TYPE::Water, std::make_shared<Texture2D>("textures/water/Block_Water.png") });
		blockTextures->insert({ TEXTURE_TYPE::FullSpecular, std::make_shared<Texture2D>("textures/Full_Specular.png") });
		blockTextures->insert({ TEXTURE_TYPE::NoSpecular, std::make_shared<Texture2D>("textures/No_Specular.png") });
	}

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
	glm::vec3 worldPosOffset{ m_worldPos.x, 0.0f, m_worldPos.y };
	for (int z = 0; z < Z_DIMENSION; ++z) {
		for (int x = 0; x < X_DIMENSION; ++x) {
			// Creates the block at a position based on noise
			int yPos = (int)(noise->GetNoise((float)x + m_worldPos.x, (float)z + m_worldPos.y) * CUBE_OFFSET) + 127;
			glm::vec3 chunkPos = glm::ivec3(x, yPos, z);
			glm::mat4 model = glm::translate(glm::mat4(1.0f), chunkPos + worldPosOffset);
			//m_pModelMatrices[position(x, yPos, z)] = model;
			//std::cout << "Inserting GRASS block at (" << x << ", " << yPos << ", " << z << ")\n";
			m_pBlocks[position(x, yPos, z)] =
				std::move(Block(
					BLOCK_TYPE::Grass,
					blockTextures->at(TEXTURE_TYPE::GrassTop),
					model,
					m_ID,
					chunkPos));

			// Fills in all the cubes above with air blocks
			for (int y = yPos + 1; y <= MAX_Y; ++y) {
				chunkPos = glm::ivec3(x, y, z);
				model = glm::translate(glm::mat4(1.0f), chunkPos + worldPosOffset);
				//m_pModelMatrices[position(x, y, z)] = model;
				//std::cout << "Inserting AIR block at (" << x << ", " << y << ", " << z << ")\n";
				m_pBlocks[position(x, y, z)] = std::move(
					Block(BLOCK_TYPE::Air,
						  blockTextures->at(TEXTURE_TYPE::Air),
						  model,
						  m_ID,
						  chunkPos));
			}

			// Fills in all the cubes below with dirt blocks
			for (int y = yPos - 1; y >= MIN_Y; --y) {
				chunkPos = glm::ivec3(x, y, z);
				model = glm::translate(glm::mat4(1.0f), chunkPos + worldPosOffset);
				//m_pModelMatrices[position(x, y, z)] = model;
				if (y + 5 < yPos) {
					//std::cout << "Inserting STONE block at (" << x << ", " << y << ", " << z << ")\n";
					m_pBlocks[position(x, y, z)] = std::move(
						Block(BLOCK_TYPE::Stone,
							  blockTextures->at(TEXTURE_TYPE::Stone),
							  model,
							  m_ID,
							  chunkPos));
				}
				else {
					//std::cout << "Inserting DIRT block at (" << x << ", " << y << ", " << z << ")\n";
					m_pBlocks[position(x, y, z)] = std::move(
						Block(BLOCK_TYPE::Dirt,
							  blockTextures->at(TEXTURE_TYPE::Dirt),
							  model,
							  m_ID,
							  chunkPos));
				}
			}
		}
	}
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

void Chunk::render(const Shader& shader) const {
	shader.use();

	Block* block;
	for (int x = 0; x < X_DIMENSION; ++x) {
		for (int y = 0; y < Y_DIMENSION; ++y) {
			for (int z = 0; z < Z_DIMENSION; ++z) {
				block = &m_pBlocks[position(x, y, z)];
				if (block == nullptr) {
					std::cout << "This block position hasn't been assigned a block yet (BUG)\n";
				}
				else if (block->getBlockType() != BLOCK_TYPE::Air) {
					block->bindTextures();
					block->bindVAO();
					glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, glm::value_ptr(block->m_model));
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
				//else {
				//	std::cout << "block (" << x << ", " << y << ", " << z << ") was either never initialized or is an air block" << std::endl;
				//}
			}
		}
	}
	block = nullptr;
	delete block;
}



Block& Chunk::getBlockAt(glm::ivec3 blockPosInChunk) const {
	int x = blockPosInChunk.x;
	int y = blockPosInChunk.y;
	int z = blockPosInChunk.z;
	return m_pBlocks[position(x,y,z)];
}
