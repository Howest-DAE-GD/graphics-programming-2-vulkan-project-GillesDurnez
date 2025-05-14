#pragma once

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include "../Time.h"
#include "Window.h"

#define PI 3.14159265358979323846f
#define TO_RADIANS 0.01745329251994329576923690768489f

// TODO: FIX ME PLS
// ROTATION IS CURSED 
namespace gp2
{
	struct Camera
	{
		Camera(Window* pWindow)
			: m_pWindow{ pWindow }
		{
			Initialize();
		}

		Camera(Window* pWindow, float aspectRatio, const glm::vec3& _origin, float _fovAngle, float near, float far)
			: m_pWindow{ pWindow }
		{
			Initialize(_fovAngle, _origin, aspectRatio, near, far);
		}

		glm::vec3 origin{ 0.f };
		float fovAngle{ 45.f };
		float fov{ 0.f };
		float aspectRatio{ 1.f };

		glm::vec3 forward{ 0.f, 0.f, 1.f };
		glm::vec3 up{ 0.f, 1.f, 0.f };
		glm::vec3 right{ 1.f, 0.f, 0.f };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };
		float totalRoll{ 0.f };

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };

		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 projectionMatrix{ 1.f };

		void Initialize(float _fovAngle = 90.f, glm::vec3 _origin = { 0.f, 0.f, 0.f }, float aspect = 1.f, float nearPl = 0.1f, float farPl = 10000.f)
		{
			fovAngle = _fovAngle;
			origin = _origin;
			aspectRatio = aspect;
			nearPlane = nearPl;
			farPlane = farPl;

			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
			CalculateProjectionMatrix();
		}

		void CalculateViewMatrix()
		{
			viewMatrix = glm::lookAt(origin, origin + forward, glm::vec3(0,1,0));
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
			projectionMatrix[1][1] *= -1; // Vulkan-specific: invert Y-axis
		}

		void Update()
		{
			const float deltaTime = gp2::Time::DeltaTime / 1000.0f;
			float MOVE_SPEED = 500.f;

			mouseDelta = GetMouseDelta();

			// Keyboard movement
			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				MOVE_SPEED *= 15;

			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_W) == GLFW_PRESS)
				origin += forward * MOVE_SPEED * deltaTime;
			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_S) == GLFW_PRESS)
				origin -= forward * MOVE_SPEED * deltaTime;
			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_D) == GLFW_PRESS)
				origin += right * MOVE_SPEED * deltaTime;
			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_A) == GLFW_PRESS)
				origin -= right * MOVE_SPEED * deltaTime;

			// Mouse rotation
			const float ROTATION_SPEED = 0.01f;

			bool leftMouse = glfwGetMouseButton(m_pWindow->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			bool rightMouse = glfwGetMouseButton(m_pWindow->GetWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

			if (leftMouse && rightMouse)
			{
				origin -= mouseDelta.y * up * MOVE_SPEED * 2.0f * deltaTime;
			}
			else if (leftMouse)
			{
				origin -= forward * mouseDelta.y * MOVE_SPEED * deltaTime;
				totalYaw += mouseDelta.x * ROTATION_SPEED;
			}
			else if (rightMouse)
			{
				totalYaw -= mouseDelta.x * ROTATION_SPEED;
				totalPitch += mouseDelta.y * ROTATION_SPEED;
				totalPitch = std::clamp(totalPitch, -glm::half_pi<float>(), glm::half_pi<float>());
			}

			// Construct rotation matrix
			glm::mat4 rotation = glm::mat4(1.f);
			rotation = glm::rotate(rotation, totalYaw, glm::vec3{ 0.f, 1.f, 0.f });    // Yaw around Y
			rotation = glm::rotate(rotation, totalPitch, glm::vec3{ 1.f, 0.f, 0.f });  // Pitch around X
			rotation = glm::rotate(rotation, totalRoll, glm::vec3{ 0.f, 0.f, 1.f });   // Roll around Z (optional)

			forward = glm::normalize(glm::vec3(rotation * glm::vec4{ 0, 0, 1, 0 }));

			// Recalculate right and up from view matrix
			CalculateViewMatrix();
			glm::mat4 inv = glm::inverse(viewMatrix);
			right = glm::vec3(inv[0]);
			up = glm::vec3(inv[1]);

			// Only recalculate projection matrix if needed (e.g., on resize or FOV change)
			// CalculateProjectionMatrix(); <-- Do only when fov or aspectRatio changes
		}

		glm::vec2 GetMouseDelta()
		{
			glm::vec2 oldMousePos = mousePos;
			double mouseX{}, mouseY{};
			glfwGetCursorPos(m_pWindow->GetWindow(), &mouseX, &mouseY);
			mousePos = { static_cast<float>(mouseX), static_cast<float>(mouseY) };
			return mousePos - oldMousePos;
		}

		glm::vec2 GetMousePos()
		{
			double mouseX{}, mouseY{};
			glfwGetCursorPos(m_pWindow->GetWindow(), &mouseX, &mouseY);
			return { static_cast<float>(mouseX), static_cast<float>(mouseY) };
		}

	private:
		Window* m_pWindow{ nullptr };
		glm::vec2 mousePos{ 0.f };
		glm::vec2 mouseDelta{ 0.f };
	};
}
