#include "Chunk.h"


#define position(x,y,z) (x + (y * X_DIMENSION) + (z * Y_DIMENSION * X_DIMENSION))

Chunk::Chunk(const glm::vec3& worldPos) : 
	m_ID(++s_ChunkIDCounter), 
	m_WorldPos(worldPos), 
	m_VisVerts(std::vector<Vertex>()),
	m_pBlocks(std::make_unique<Block[]>(VOLUME)),
	m_Model(glm::translate(glm::mat4(1.0f), m_WorldPos)) {
}

void Chunk::generateVAOandVBO() {
	glGenVertexArrays(1, &m_Vao);
	glGenBuffers(1, &m_Vbo);
}

// Stores the vertex data of the visible faces into the buffer
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

int Chunk::getID() {
	return m_ID;
}

void Chunk::render(const Shader& shader) const {
	shader.use();
	// Causes the chunk the be drawn centered at its world position
	glm::mat4 model = glm::translate(m_Model, glm::vec3(-X_DIMENSION / 2, 0.0f, -Z_DIMENSION / 2));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(m_Vao);
	glDrawArrays(GL_TRIANGLES, 0, m_NumVisFaces * Block::VERTS_PER_FACE);
}

// WITH away-face culling
//void Chunk::updateVisibleFacesMesh(const glm::vec3& cameraPos) {
//	//bool backVis = false, frontVis = false, leftVis = false, rightVis = false, bottomVis = false, topVis = false;
//	m_NumVisFaces = 0;
//	for (int z = 0; z < Z_DIMENSION; ++z) {
//		for (int y = 0; y < Y_DIMENSION; ++y) {
//			for (int x = 0; x < X_DIMENSION; ++x) {
//				//updateVFM(x, y, z);
//				Block* block = &m_pBlocks[position(x, y, z)];
//				if (block->m_IsSolid) {
//					int texType = static_cast<int>(block->getTexType());
//					// Appends left face if true
//					if (((x > 0 && !m_pBlocks[position((x - 1), y, z)].m_IsSolid) || x == 0)) {
//						glm::vec3 norm = glm::vec3(-1.0f, 0.0f, 0.0f);
//						glm::vec3 facePos{ block->getWorldPos() + glm::vec3(-0.5f, 0.0f, 0.0f) };
//						if (glm::dot(glm::normalize(cameraPos - facePos), norm) > 0) {
//							//std::cout << "Appending left face\n";
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(-1.0f, 0.0f, 0.0f),
//									glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(-1.0f, 0.0f, 0.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(-1.0f, 0.0f, 0.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(-1.0f, 0.0f, 0.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(-1.0f, 0.0f, 0.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(-1.0f, 0.0f, 0.0f),
//									glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//
//							++m_NumVisFaces;
//						}
//					}
//					// Appends right face if true
//					if ((x < X_DIMENSION - 1 && !m_pBlocks[position((x + 1), y, z)].m_IsSolid) || x == X_DIMENSION - 1) {
//						glm::vec3 norm = glm::vec3(1.0f, 0.0f, 0.0f);
//						glm::vec3 facePos{ block->getWorldPos() + glm::vec3(0.5f, 0.0f, 0.0f) };
//						if (glm::dot(glm::normalize(cameraPos - facePos), norm) > 0) {
//							//std::cout << "Appending right face\n";
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(1.0f, 0.0f, 0.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(1.0f, 0.0f, 0.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(1.0f, 0.0f, 0.0f),
//									glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(1.0f, 0.0f, 0.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(1.0f, 0.0f, 0.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(1.0f, 0.0f, 0.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//
//							++m_NumVisFaces;
//						}
//					}
//					// Appends bottom face if true
//					if ((y > 0 && !m_pBlocks[position(x, (y - 1), z)].m_IsSolid) || y == 0) {
//						glm::vec3 norm = glm::vec3(0.0f, -1.0f, 0.0f);
//						glm::vec3 facePos{ block->getWorldPos() + glm::vec3(0.0f, -0.5f, 0.0f) };
//						if (glm::dot(glm::normalize(cameraPos - facePos), norm) > 0) {
//							//std::cout << "Appending bottom face\n";
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(0.0f, -1.0f, 0.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(0.0f, -1.0f, 0.0f),
//									glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(0.0f, -1.0f, 0.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(0.0f, -1.0f, 0.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(0.0f, -1.0f, 0.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(0.0f, -1.0f, 0.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//
//							++m_NumVisFaces;
//						}
//					}
//					// Appends top face if true
//					if ((y < Y_DIMENSION - 1 && !m_pBlocks[position(x, (y + 1), z)].m_IsSolid) || y == Y_DIMENSION - 1) {
//						glm::vec3 norm = glm::vec3(0.0f, 1.0f, 0.0f);
//						glm::vec3 facePos{ block->getWorldPos() + glm::vec3(0.0f, 0.5f, 0.0f) };
//						if (glm::dot(glm::normalize(cameraPos - facePos), norm) > 0) {
//							//std::cout << "Appending top face\n";
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  1.0f,  0.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  1.0f,  0.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  1.0f,  0.0f),
//									glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  1.0f,  0.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  1.0f,  0.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  1.0f,  0.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//
//							++m_NumVisFaces;
//						}
//					}
//					// Appends back face if true
//					if ((z > 0 && !m_pBlocks[position(x, y, (z - 1))].m_IsSolid) || z == 0) {
//						glm::vec3 norm = glm::vec3(0.0f, 0.0f, -1.0f);
//						glm::vec3 facePos{ block->getWorldPos() + glm::vec3(0.0f, 0.0f, -0.5f) };
//						if (glm::dot(glm::normalize(cameraPos - facePos), norm) > 0) {
//							//std::cout << "Appending back face\n";
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  0.0f, -1.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  0.0f, -1.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  0.0f, -1.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  0.0f, -1.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  0.0f, -1.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y, -0.5f + z),
//									glm::vec3(0.0f,  0.0f, -1.0f),
//									glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//
//							++m_NumVisFaces;
//						}
//					}
//					// Append front face if true
//					if ((z < Z_DIMENSION - 1 && !m_pBlocks[position(x, y, (z + 1))].m_IsSolid) || z == Z_DIMENSION - 1) {
//						glm::vec3 norm = glm::vec3(0.0f, 0.0f, 1.0f);
//						glm::vec3 facePos{ block->getWorldPos() + glm::vec3(0.0f, 0.0f, 0.5f) };
//						if (glm::dot(glm::normalize(cameraPos - facePos), norm) > 0) {
//							//std::cout << "Appending front face\n";
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  0.0f, 1.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//							// bottom-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  0.0f, 1.0f),
//									glm::vec3(1.0f, 0.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{ glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
//								glm::vec3(0.0f,  0.0f, 1.0f),
//								glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//							// top-right
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  0.0f, 1.0f),
//									glm::vec3(1.0f, 1.0f, texType)
//								}
//							);
//							// top-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x,  0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  0.0f, 1.0f),
//									glm::vec3(0.0f, 1.0f, texType)
//								}
//							);
//							// bottom-left
//							m_VisVerts.push_back(
//								Vertex{
//									glm::vec3(-0.5f + x, -0.5f + y,  0.5f + z),
//									glm::vec3(0.0f,  0.0f, 1.0f),
//									glm::vec3(0.0f, 0.0f, texType)
//								}
//							);
//
//							++m_NumVisFaces;
//						}
//					}
//				}
//			}
//		}
//	}
//}

Chunk::Chunk(Chunk&& chunk) noexcept :
	m_ID(std::move(chunk.m_ID)),
	m_WorldPos(std::move(chunk.m_WorldPos)),
	m_VisVerts(std::move(chunk.m_VisVerts)),
	m_pBlocks(std::move(chunk.m_pBlocks)),
	m_Vao(std::move(chunk.m_Vao)),
	m_Vbo(std::move(chunk.m_Vbo)),
	m_NumVisFaces(std::move(chunk.m_NumVisFaces)), 
	m_Model(std::move(chunk.m_Model)) {

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
		this->m_Model = std::move(chunk.m_Model);
	}

	return *this;
}