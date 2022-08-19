#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Shader.h"
#include "Texture2D.h"
#include "Camera.h"
#include "ChunkManager.h"

#pragma region Function_Declarations
void processInput(GLFWwindow* window);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mousePosCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void createArrayTexture(unsigned int& arrTex);
GLFWwindow* windowSetup();
void genCubeVAOandVBO(unsigned int& cubeVAO, unsigned int& cubeVBO);
#pragma endregion

#pragma region Global_Variables
int screenWidth = 800;
int screenHeight = 600;

int monitorResX = 1920;
int monitorResY = 1080;

const char* TITLE = "Voxel Tech Demo";

double lastXPos = screenWidth / 2;
double lastYPos = screenHeight / 2;

float deltaTime = 0.0f;

bool firstMouseMove = true;
bool flashlightIsActive = false;

const glm::mat4 IDENTITY{ 1.0f };

const int TEX_ARR_UNIT = 1;
const int REFRESH_RATE = 200;

glm::mat4 view;
glm::mat4 projection;

GLfloat screenQuadVertices[] = {
	// Positions	// Tex coords
	-1.0, -1.0,		 0.0,  0.0,
	 1.0, -1.0,		 1.0,  0.0,
	 1.0,  1.0,		 1.0,  1.0,

	-1.0, -1.0,		 0.0,  0.0,
	 1.0,  1.0,		 1.0,  1.0,
	-1.0,  1.0,		 0.0,  1.0
};
float cubeVertices[] = {
	// Positions           // Normals      // Tex Coords
	// Back face
	-1.0f, -1.0f, -1.0f,    0.0f,  0.0f,   -1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f,  1.0f, -1.0f,    0.0f,  0.0f,   -1.0f, 1.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,    0.0f,  0.0f,   -1.0f, 1.0f, 0.0f, // bottom-right         
	 1.0f,  1.0f, -1.0f,    0.0f,  0.0f,   -1.0f, 1.0f, 1.0f, // top-right
	-1.0f, -1.0f, -1.0f,    0.0f,  0.0f,   -1.0f, 0.0f, 0.0f, // bottom-left
	-1.0f,  1.0f, -1.0f,    0.0f,  0.0f,   -1.0f, 0.0f, 1.0f, // top-left
	// Front face		  				  
	-1.0f, -1.0f,  1.0f,    0.0f,  0.0f,    1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,    0.0f,  0.0f,    1.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,    0.0f,  0.0f,    1.0f, 1.0f, 1.0f, // top-right
	 1.0f,  1.0f,  1.0f,    0.0f,  0.0f,    1.0f, 1.0f, 1.0f, // top-right
	-1.0f,  1.0f,  1.0f,    0.0f,  0.0f,    1.0f, 0.0f, 1.0f, // top-left
	-1.0f, -1.0f,  1.0f,    0.0f,  0.0f,    1.0f, 0.0f, 0.0f, // bottom-left
	// Left face		  				  
	-1.0f,  1.0f,  1.0f,   -1.0f,  0.0f,    0.0f, 1.0f, 0.0f, // top-right
	-1.0f,  1.0f, -1.0f,   -1.0f,  0.0f,    0.0f, 1.0f, 1.0f, // top-left
	-1.0f, -1.0f, -1.0f,   -1.0f,  0.0f,    0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f,   -1.0f,  0.0f,    0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,   -1.0f,  0.0f,    0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f,  1.0f,  1.0f,   -1.0f,  0.0f,    0.0f, 1.0f, 0.0f, // top-right
	// Right face		  				  
	 1.0f,  1.0f,  1.0f,    1.0f,  0.0f,    0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f, -1.0f,    1.0f,  0.0f,    0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,    1.0f,  0.0f,    0.0f, 1.0f, 1.0f, // top-right         
	 1.0f, -1.0f, -1.0f,    1.0f,  0.0f,    0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,    1.0f,  0.0f,    0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f,  1.0f,    1.0f,  0.0f,    0.0f, 0.0f, 0.0f, // bottom-left     
	// Bottom face		  				  
	-1.0f, -1.0f, -1.0f,    0.0f, -1.0f,    0.0f, 0.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,    0.0f, -1.0f,    0.0f, 1.0f, 1.0f, // top-left
	 1.0f, -1.0f,  1.0f,    0.0f, -1.0f,    0.0f, 1.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,    0.0f, -1.0f,    0.0f, 1.0f, 0.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,    0.0f, -1.0f,    0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f, -1.0f, -1.0f,    0.0f, -1.0f,    0.0f, 0.0f, 1.0f, // top-right
	// Top face			  				  
	-1.0f,  1.0f, -1.0f,    0.0f,  1.0f,    0.0f, 0.0f, 1.0f, // top-left
	 1.0f,  1.0f , 1.0f,    0.0f,  1.0f,    0.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,    0.0f,  1.0f,    0.0f, 1.0f, 1.0f, // top-right     
	 1.0f,  1.0f,  1.0f,    0.0f,  1.0f,    0.0f, 1.0f, 0.0f, // bottom-right
	-1.0f,  1.0f, -1.0f,    0.0f,  1.0f,    0.0f, 0.0f, 1.0f, // top-left
	-1.0f,  1.0f,  1.0f,    0.0f,  1.0f,    0.0f, 0.0f, 0.0f  // bottom-left        
};

Camera camera{ glm::vec3(0.0f, 50.0f, 0.0f) };
#pragma endregion

int main() {
	GLFWwindow* window = windowSetup();

	#pragma region Shaders
	Shader basicShader{ "shaders/basic.vert", "shaders/basic.frag" };
	Shader debugChunkShader{ "shaders/debug_chunk.vert", "shaders/debug_chunk.frag" };
	#pragma endregion

	stbi_set_flip_vertically_on_load(true);

	unsigned int cubeVAO;
	unsigned int cubeVBO;
	genCubeVAOandVBO(cubeVAO, cubeVBO);

	// Constructs array texture
	unsigned int arrTex;
	createArrayTexture(arrTex);

	float lastFrame = 0.0f;
	float currFrame = 0.0f;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// OLD Render Loop
	/*
	while (false) {
		currFrame = static_cast<float>(glfwGetTime());
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;

		processInput(window);

		glClearColor(0.2f, 0.5f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)screenWidth / (float)screenHeight, 0.1f, 2000.0f);

		#pragma region Point_Light_Drawing
		//lightShader.use();

		//glUniformMatrix4fv(glGetUniformLocation(lightShader.id, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//glUniformMatrix4fv(glGetUniformLocation(lightShader.id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		//glBindVertexArray(lightVAO);
		//for (int i = 0; i < 4; ++i) {
		//	glm::mat4 lightModel = glm::mat4(1.0f);
		//	lightModel = glm::translate(lightModel, lightPositions[i]);
		//	lightModel = glm::scale(lightModel, glm::vec3(0.2f, 0.2f, 0.2f));
		//	glUniformMatrix4fv(glGetUniformLocation(lightShader.id, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
		//	glUniform3fv(glGetUniformLocation(lightShader.id, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

		//	glDrawArrays(GL_TRIANGLES, 0, 36);
		//}
		#pragma endregion

		#pragma region Sunlight_Drawing
		// Rotate around NUM_CUBES_X / 2 axis
		glm::vec3 sunColor{ 1.0f, 1.0f, 1.0f };
		glm::vec3 sunDistance{ 1500, 1500, 0 };
		glm::mat4 sunModel{ 1.0f };
		sunModel = glm::translate(sunModel, 
								  glm::vec3(glm::sin(glm::radians(glfwGetTime() * 20)) * sunDistance.x, 
											glm::cos(glm::radians(glfwGetTime() * 20)) * sunDistance.y, 
											-NUM_CUBES_Z / 2));
		sunModel = glm::scale(sunModel, glm::vec3(120.0f));
		glm::vec3 sunPos = sunModel[3];
		// sunLightColor is (255, 200, 117) at sunrise/sunset, and (255, 255, 240) at its peak
		//glm::vec3 sunLightColor{ 255, 255.0f/(sunDistance.y / sunPos.y), 240.0f / (sunDistance.y / sunPos.y) };
		glm::vec3 sunLightColor{ 1.0f, 1.0f, 240.0f / 255.0f };
		//std::cout << sunPos.x << ", " << sunPos.y << ", " << sunPos.z << std::endl;

		glUniformMatrix4fv(glGetUniformLocation(lightShader.id, "model"), 1, GL_FALSE, glm::value_ptr(sunModel));
		glUniform3fv(glGetUniformLocation(lightShader.id, "lightColor"), 1, glm::value_ptr(sunColor));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		#pragma endregion

		#pragma region Moonlight_Drawing
		// Rotate around NUM_CUBES_X / 2 axis
		glm::vec3 moonColor{ 168.0f / 255.0f, 178.0f / 255.0f, 193.0f / 255.0f };
		glm::vec3 moonDistance{ -1000, -1000, 0 };
		glm::mat4 moonModel{ 1.0f };
		moonModel = glm::translate(moonModel, 
								   glm::vec3(glm::sin(glm::radians(glfwGetTime() * 20)) * moonDistance.x,
											 glm::cos(glm::radians(glfwGetTime() * 20)) * moonDistance.y,
											 -NUM_CUBES_Z / 2));
		moonModel = glm::scale(moonModel, glm::vec3(80.0f));
		glm::vec3 moonPos = moonModel[3];
		//glm::vec3 moonLightColor{ 60, 100.0f / (moonDistance.y / moonPos.y), 140.0f / (moonDistance.y / moonPos.y) };
		glm::vec3 moonLightColor{ 60.0f / 255.0f, 100.0f / 255.0f, 140.0f / 255.0f };
		glUniformMatrix4fv(glGetUniformLocation(lightShader.id, "model"), 1, GL_FALSE, glm::value_ptr(moonModel));
		glUniform3fv(glGetUniformLocation(lightShader.id, "lightColor"), 1, glm::value_ptr(moonColor));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		#pragma endregion

		#pragma region Terrain_Drawing
		objShader.use();

		objDiffuseMap.bind(0);
		objSpecularMap.bind(1);

		#pragma region Shader_Uniforms
		glUniformMatrix4fv(glGetUniformLocation(objShader.id, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(objShader.id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3fv(glGetUniformLocation(objShader.id, "viewPos"), 1, glm::value_ptr(camera.cameraPosition));

		glUniform1i(glGetUniformLocation(objShader.id, "material.diffuse"), 0);
		glUniform1i(glGetUniformLocation(objShader.id, "material.specular"), 1);
		glUniform1f(glGetUniformLocation(objShader.id, "material.shininess"), 8.0f);

		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[0].direction"), 1, glm::value_ptr(sunPos));
		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[0].ambient"), 1, glm::value_ptr(glm::vec3(0.2f) * sunLightColor));
		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[0].diffuse"), 1, glm::value_ptr(glm::vec3(0.5f) * sunLightColor));
		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[0].specular"), 1, glm::value_ptr(glm::vec3(0.05f) * sunLightColor));

		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[1].direction"), 1, glm::value_ptr(moonPos));
		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[1].ambient"), 1, glm::value_ptr(glm::vec3(0.2f) * moonLightColor));
		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[1].diffuse"), 1, glm::value_ptr(glm::vec3(1.0f) * moonLightColor));
		glUniform3fv(glGetUniformLocation(objShader.id, "dirLight[1].specular"), 1, glm::value_ptr(glm::vec3(0.03f) * moonLightColor));

		for (int i = 0; i < 4; ++i) {
			std::string index = std::to_string(i);
			glUniform3fv(glGetUniformLocation(objShader.id, ("pointLight[" + index + "].position").c_str()), 1,
						 glm::value_ptr(lightPositions[i]));
			glUniform1f(glGetUniformLocation(objShader.id, ("pointLight[" + index + "].constant").c_str()), 1.0f);
			glUniform1f(glGetUniformLocation(objShader.id, ("pointLight[" + index + "].linear").c_str()), 0.14f);
			glUniform1f(glGetUniformLocation(objShader.id, ("pointLight[" + index + "].quadratic").c_str()), 0.07f);
			glUniform3fv(glGetUniformLocation(objShader.id, ("pointLight[" + index + "].ambient").c_str()), 1,
						 glm::value_ptr(glm::vec3(0.03f)));
			glUniform3fv(glGetUniformLocation(objShader.id, ("pointLight[" + index + "].diffuse").c_str()), 1,
						 glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
			glUniform3fv(glGetUniformLocation(objShader.id, ("pointLight[" + index + "].specular").c_str()), 1,
						 glm::value_ptr(glm::vec3(0.05f)));
		}

		glUniform1i(glGetUniformLocation(objShader.id, "spotLight.isActive"), flashlightIsActive);
		glUniform3fv(glGetUniformLocation(objShader.id, "spotLight.position"), 1, glm::value_ptr(camera.cameraPosition));
		glUniform3fv(glGetUniformLocation(objShader.id, "spotLight.direction"), 1, glm::value_ptr(camera.cameraFront));
		glUniform1f(glGetUniformLocation(objShader.id, "spotLight.innerCutoff"), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(objShader.id, "spotLight.outerCutoff"), glm::cos(glm::radians(17.5f)));
		glUniform1f(glGetUniformLocation(objShader.id, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(objShader.id, "spotLight.linear"), 0.07f);
		glUniform1f(glGetUniformLocation(objShader.id, "spotLight.quadratic"), 0.017f);
		glUniform3fv(glGetUniformLocation(objShader.id, "spotLight.ambient"), 1, glm::value_ptr(glm::vec3(0.1f)));
		glUniform3fv(glGetUniformLocation(objShader.id, "spotLight.diffuse"), 1, glm::value_ptr(glm::vec3(0.5f)));
		glUniform3fv(glGetUniformLocation(objShader.id, "spotLight.specular"), 1, glm::value_ptr(glm::vec3(0.05f)));
		#pragma endregion

		glBindVertexArray(objVAO);
		//int noiseInd = 0;
		//for (int i = 0; i < NUM_CUBES_X; ++i) {
		//	for (int j = 0; j < NUM_CUBES_Z; ++j) {
		//		glm::mat4 model = glm::mat4(1.0f);
		//		model = glm::translate(model, glm::vec3(i, (int)(noiseData[noiseInd++] * 10), -j));
		//		glUniformMatrix4fv(glGetUniformLocation(objShader.id, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//		glDrawArrays(GL_TRIANGLES, 0, 36);
		//	}
		//}
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, TOTAL_NUM_CUBES);
		#pragma endregion

		//drawTerrain(objShader, objVAO, objDiffuseMap, objSpecularMap);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	*/

	ChunkManager chunkManager{ camera.m_Position };

	// NEW Render Loop
	while (!glfwWindowShouldClose(window)) {
		currFrame = static_cast<float>(glfwGetTime());
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;
		//std::cout << "Frame Rate: " << 1 / deltaTime << std::endl;
		//std::cout << "Camera Pos: (" << camera.m_Position.x << ", " << camera.m_Position.y << ", " << camera.m_Position.z << ")\n";

		processInput(window);

		glClearColor(0.2f, 0.5f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.m_Fov), (float)screenWidth / (float)screenHeight, 0.1f, 2000.0f);

		#pragma region Terrain_Drawing
		glActiveTexture(GL_TEXTURE0 + TEX_ARR_UNIT);
		glBindTexture(GL_TEXTURE_2D_ARRAY, arrTex);
		// Shader Uniforms
		glUniformMatrix4fv(glGetUniformLocation(basicShader.id, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShader.id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(debugChunkShader.id, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(debugChunkShader.id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(glGetUniformLocation(basicShader.id, "arrTex"), TEX_ARR_UNIT);
		glUniform3fv(glGetUniformLocation(basicShader.id, "cameraPos"), 1, glm::value_ptr(camera.m_Position));
		
		chunkManager.recalculate(camera.m_Position);
		//std::cout << "Rendering chunks..." << std::endl;
		chunkManager.renderChunks(basicShader);
		//std::cout << "Finished rendering chunks." << std::endl;
		//chunkManager.renderChunkCenters(debugChunkShader, cubeVAO);
		#pragma endregion

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Delete any buffers created in the render loop HERE

	glfwTerminate();

	return 0;
}

std::vector<std::future<void>> futures;
std::mutex chunksMutex;

void processInput(GLFWwindow* window) {
	// Close window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// Fullscreen
	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
		// Sets the window to display in fullscreen on the primary monitor
		const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		screenWidth = vidMode->width;
		screenHeight = vidMode->height;
		glfwSetWindowMonitor(window, NULL, 0, 0, screenWidth, screenHeight, REFRESH_RATE);
		glViewport(0, 0, screenWidth, screenHeight);
	}
	// Windowed
	if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS) {
		glfwSetWindowMonitor(window, NULL, 50, 50, 800, 600, REFRESH_RATE);
		glViewport(0, 0, 800, 600);
	}

	// Enable/Disable visible edge drawing
	if (glfwGetKey(window, GLFW_KEY_BACKSLASH) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(window, GLFW_KEY_BACKSLASH) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Enable/Disable flashlight
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		flashlightIsActive = true;
		//std::cout << "Flashlight is active? " << flashlightIsActive << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		flashlightIsActive = false;
		//std::cout << "Flashlight is active? " << flashlightIsActive << std::endl;
	}

	// Camera Controls
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.input(CAMERA_MOVEMENT::FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.input(CAMERA_MOVEMENT::BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.input(CAMERA_MOVEMENT::LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.input(CAMERA_MOVEMENT::RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.input(CAMERA_MOVEMENT::UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		camera.input(CAMERA_MOVEMENT::DOWN, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.applySpeedMultiplier();
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		camera.removeSpeedMultiplier();
	}
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	screenWidth = width;
	screenHeight = height;
	glViewport(0, 0, width, height);
}

void mousePosCallback(GLFWwindow* window, double xPos, double yPos) {
	if (firstMouseMove) {
		lastXPos = xPos;
		lastYPos = yPos;
		firstMouseMove = false;
	}

	float xOffset = static_cast<float>(xPos - lastXPos);
	float yOffset = static_cast<float>(yPos - lastYPos);
	lastXPos = xPos;
	lastYPos = yPos;
	
	camera.processMouseMove(xOffset, yOffset);
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	camera.processMouseScroll(static_cast<float>(yOffset));
}

void createArrayTexture(unsigned int& arrTex) {
	const int NUM_TEXTURES = 6;
	std::string texDirs[NUM_TEXTURES] = {
		{ "textures/grass/Block_Grass_Top.png" },
		{ "textures/grass/Block_Grass_Side.png" },
		{ "textures/dirt/Block_Dirt.png" },
		{ "textures/stone/Block_Stone.png" },
		{ "textures/sand/Block_Sand.png" },
		{ "textures/water/Block_Water.png" },
		//{ "textures/Full_Specular.png", false },
		//{ "textures/No_Specular.png", false }
	};

	glGenTextures(1, &arrTex);
	glActiveTexture(GL_TEXTURE0 + TEX_ARR_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, arrTex);

	const int TEX_ARR_WIDTH = 16; // # pixels
	const int TEX_ARR_HEIGHT = 16; // # pixels
	const int TEX_ARR_DEPTH = 16; // # textures
	const int TEX_ARR_WIDTH_BYTES = TEX_ARR_WIDTH * sizeof(unsigned char);
	const int TEX_ARR_HEIGHT_BYTES = TEX_ARR_HEIGHT * sizeof(unsigned char);

	int texWidth, texHeight, texNumChannels;
	std::vector<unsigned char> pixelData = std::vector<unsigned char>();
	// Populates the pixel data vector for creating a 2D array texture
	for (int i = 0; i < NUM_TEXTURES; ++i) {
		unsigned char* texData = stbi_load(texDirs[i].c_str(), &texWidth, &texHeight, &texNumChannels, 0);
		if (texData) {
			for (int channel = 0; channel < texNumChannels; ++channel) {
				for (int y = 0; y < texHeight; ++y) {
					for (int x = 0; x < texWidth; ++x) {
						pixelData.emplace_back(std::move(texData[x + (y * TEX_ARR_WIDTH) + (channel * TEX_ARR_WIDTH * TEX_ARR_HEIGHT)]));
					}
				}
			}
			stbi_image_free(texData);
		}
		else {
			std::cout << "Could not load texture data\n";
		}
	}

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 16, 16, TEX_ARR_DEPTH,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	pixelData.clear();

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glActiveTexture(GL_TEXTURE0);
}

GLFWwindow* windowSetup() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, TITLE, NULL, NULL);

	if (!window) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);

	// GLFW callbacks
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetScrollCallback(window, scrollCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load GLAD" << std::endl;
		glfwTerminate();
		exit(-1);
	}

	glViewport(0, 0, screenWidth, screenHeight);

	return window;
}

void genCubeVAOandVBO(unsigned int& cubeVAO, unsigned int& cubeVBO) {
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 5));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}