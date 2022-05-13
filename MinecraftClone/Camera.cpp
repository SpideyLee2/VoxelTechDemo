#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up) : cameraPosition(pos), cameraFront(front), worldUp(up) {
	yaw = -90.0f;
	pitch = 0.0f;

	mouseSensitivity = 0.1f;
	scrollSensitivity = 1.5f;

	speed = 10.0f;
	speedMultiplier = 3.0f;
	tempSpeed = speed;

	fov = 45.0f;

	pitchConstraintHi = 89.0f;
	pitchConstraintLo = -89.0f;
	fovConstraintHi = 90.0f;
	fovConstraintLo = 10.0f;
}

glm::mat4 Camera::getViewMatrix() {
	return glm::lookAt(cameraPosition, cameraPosition + cameraFront, worldUp);
}

void Camera::input(CAMERA_MOVEMENT movementDirection, float deltaTime) {
	float velocity = tempSpeed * deltaTime;
	if (movementDirection == CAMERA_MOVEMENT::FORWARD) {
		cameraPosition += cameraFront * velocity;
	}
	if (movementDirection == CAMERA_MOVEMENT::BACKWARD) {
		cameraPosition -= cameraFront * velocity;
	}
	if (movementDirection == CAMERA_MOVEMENT::LEFT) {
		cameraPosition -= glm::normalize(glm::cross(cameraFront, worldUp)) * velocity;
	}
	if (movementDirection == CAMERA_MOVEMENT::RIGHT) {
		cameraPosition += glm::normalize(glm::cross(cameraFront, worldUp)) * velocity;
	}
	if (movementDirection == CAMERA_MOVEMENT::UP) {
		cameraPosition += worldUp * velocity;
	}
	if (movementDirection == CAMERA_MOVEMENT::DOWN) {
		cameraPosition -= worldUp * velocity;
	}
}

void Camera::applySpeedMultiplier() {
	tempSpeed = speed * speedMultiplier;
}

void Camera::removeSpeedMultiplier() {
	tempSpeed = speed;
}

void Camera::processMouseMove(float xOffset, float yOffset) {
	yaw += xOffset * mouseSensitivity;
	pitch += yOffset * mouseSensitivity;

	if (pitch > pitchConstraintHi)
		pitch = pitchConstraintHi;
	else if (pitch < pitchConstraintLo)
		pitch = pitchConstraintLo;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(-pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void Camera::processMouseScroll(float yOffset) {
	fov -= yOffset * scrollSensitivity;

	if (fov > fovConstraintHi)
		fov = fovConstraintHi;
	else if (fov < fovConstraintLo)
		fov = fovConstraintLo;
}

glm::mat4 Camera::myLookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 wUp) {
	glm::vec3 cDirection = glm::normalize(pos - target);
	glm::vec3 cRight = glm::normalize(glm::cross(wUp, cDirection));
	glm::vec3 cUp = glm::cross(cDirection, cRight);

	glm::mat4 rotation = glm::mat4(cRight.x, cUp.x, cDirection.x, 0.0f,
								   cRight.y, cUp.y, cDirection.y, 0.0f,
								   cRight.z, cUp.z, cDirection.z, 0.0f,
								   0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 translation = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
									  0.0f, 1.0f, 0.0f, 0.0f,
									  0.0f, 0.0f, 1.0f, 0.0f,
									  -pos.x, -pos.y, -pos.z, 1.0f);

	return rotation * translation;
}