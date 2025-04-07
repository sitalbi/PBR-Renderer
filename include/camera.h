#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <entity.h>

class Camera
{
public:

	Camera();
	Camera(int width, int height, glm::vec3 up, float yaw, float pitch);
	~Camera();

	void setFov(float fov);
	void setDistance(float distance);

	glm::mat4 getViewMatrix() const { 
		if (m_target) {
			return glm::lookAt(m_position, m_target->position, m_up);
		}
		else {
			return glm::lookAt(m_position, m_position + m_forward, m_up);
		}
			
	}
	glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(m_fov), (float)m_width / (float)m_height, m_nearClippingPlane, m_farClippingPlane); }
	glm::vec3 getPosition() const { return m_position; }

	glm::vec3 getForward() const { return m_forward; }
	glm::vec3 getRight() const { return m_right; }
	glm::vec3 getUp() const { return m_up; }

	float getYaw() const { return m_yaw; }
	float getPitch() const { return m_pitch; }
	float getFov() const { return m_fov; }
	float getDistance() const { return m_distance; }

	std::shared_ptr<Entity> getTarget() { return m_target; }
	bool hasTarget() { return m_target != nullptr; }

	void setPosition(glm::vec3 position);
	void setTarget(std::shared_ptr<Entity> target);

	void updateCameraVectors();

	void lookRotate(float deltaTime, float xoffset, float yoffset);

	void moveForward(float deltaTime);
	void moveBackward(float deltaTime);
	void moveRight(float deltaTime);
	void moveLeft(float deltaTime);
	void moveUp(float deltaTime);
	void moveDown(float deltaTime);

	void zoom(float yoffset);

	double lastX;
	double lastY;

private:
	unsigned int m_width, m_height;

	float m_sensivity = 10.0f;

	glm::vec3 m_position;
	glm::vec3 m_forward;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_worldUp;

	std::shared_ptr<Entity> m_target;
	float m_distance = 10.0f;

	float m_yaw;
	float m_pitch;
	float m_fov = 60.0f;

	float m_farClippingPlane = 100.0f;
	float m_nearClippingPlane = 0.1f;

	float m_speed = 3.0f;

	float m_minDistance = 0.75f;
	float m_maxDistance = 100.0f;

};