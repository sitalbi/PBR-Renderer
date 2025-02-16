#pragma once

#include "glm/glm.hpp"

class Camera
{
public:

	Camera();
	Camera(int width, int height, glm::vec3 up, float yaw, float pitch);
	~Camera();

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::vec3 getPosition() const;

	glm::vec3 getForward() const;
	glm::vec3 getRight() const;
	glm::vec3 getUp() const;

	void setPosition(glm::vec3 position);

	void processMouseMovement(float xoffset, float yoffset);

	void updateCameraVectors();


private:
	unsigned int m_width, m_height;

	glm::vec3 m_position;
	glm::vec3 m_forward;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_worldUp;

	float m_yaw;
	float m_pitch;
	float m_fov = 60.0f;

	float m_farClippingPlane = 100.0f;
	float m_nearClippingPlane = 0.1f;
};