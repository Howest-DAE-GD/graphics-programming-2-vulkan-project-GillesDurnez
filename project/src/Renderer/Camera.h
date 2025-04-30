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

		Camera(Window* pWindow, float aspectRatio ,const glm::vec3& _origin, float _fovAngle, float near, float far) :
			m_pWindow{ pWindow },
			origin{ _origin },
			fovAngle{ _fovAngle },
			nearPlane{ near },
			farPlane{ far }
		{
			Initialize(_fovAngle, _origin, aspectRatio, near, far);
		}


		glm::vec3 origin{};
		float fovAngle{ 45.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float aspectRatio{ 1 };

		glm::vec3 forward{ 0,0,1 };
		glm::vec3 up{ 0,1,0 };
		glm::vec3 right{ 1, 0 , 0 };

		float totalPitch{};
		float totalYaw{};
		float totalRoll{};


		float nearPlane{ .1f };
		float farPlane{ 100.f };

		glm::mat4 viewMatrix{};
		glm::mat4 projectionMatrix{ glm::perspective(fov, aspectRatio, nearPlane, farPlane) };


		void Initialize(float _fovAngle = 90.f, glm::vec3 _origin = { 0.f,0.f,0.f }, float aspectRatio = { 1 }, float nearPl = { .1f }, float farPl = { 100.f })
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			this->aspectRatio = aspectRatio;

			nearPlane = nearPl;
			farPlane = farPl;
		}

		void CalculateViewMatrix()
		{
			viewMatrix = glm::lookAt(origin, origin + forward, glm::vec3{0,0,1});
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
			projectionMatrix[1][1] *= -1; // Invert Y axis
		}

		void Update()
		{
			//Camera Update Logic

			const float deltaTime = gp2::Time::DeltaTime / 1000;
			float MOVE_SPEED = 500.f;

			// Update mouse movement
			mouseDelta = GetMouseDelta();

			//Keyboard Input

			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_LEFT_SHIFT))
			{
				MOVE_SPEED *= 5;
			}

			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_W))
			{
				origin += forward * MOVE_SPEED * deltaTime;
			}
			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_S))
			{
				origin -= forward * MOVE_SPEED * deltaTime;
			}
			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_D))
			{
				origin += right * MOVE_SPEED * deltaTime;
			}
			if (glfwGetKey(m_pWindow->GetWindow(), GLFW_KEY_A))
			{
				origin -= right * MOVE_SPEED * deltaTime;
			}

			//Mouse Input
			const float ROTATION_SPEED = 0.01f;


			if (glfwGetMouseButton(m_pWindow->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) && glfwGetMouseButton(m_pWindow->GetWindow(), GLFW_MOUSE_BUTTON_RIGHT))
			{
				origin -= mouseDelta.y * up * MOVE_SPEED * 2.f * deltaTime;
			}
			else if (glfwGetMouseButton(m_pWindow->GetWindow(), GLFW_MOUSE_BUTTON_LEFT))
			{
				origin -= forward * mouseDelta.y * MOVE_SPEED * deltaTime;
				//origin += right * mouseX * deltaTime;

				totalYaw += mouseDelta.x * ROTATION_SPEED;

				glm::mat rotation = glm::mat4(1.f);

				rotation = glm::rotate(rotation, totalYaw, glm::vec3{0,0,1});
				rotation = glm::rotate(rotation, totalPitch, glm::vec3{ 1,0,0 });
				rotation = glm::rotate(rotation, totalRoll, glm::vec3{ 0,1,0 });

				forward = glm::normalize(rotation * glm::vec4{ 0,0,1,0 });
			}
			else if (glfwGetMouseButton(m_pWindow->GetWindow(), GLFW_MOUSE_BUTTON_RIGHT))
			{
				totalYaw += mouseDelta.x * ROTATION_SPEED;
				totalPitch -= mouseDelta.y * ROTATION_SPEED;

				totalPitch = std::clamp(totalPitch, -1.57f, 1.57f);

				glm::mat rotation = glm::mat4(1.f);

				rotation = glm::rotate(rotation, totalYaw, glm::vec3{ 0,0,1 });
				rotation = glm::rotate(rotation, totalPitch, glm::vec3{ 1,0,0 });
				rotation = glm::rotate(rotation, totalRoll, glm::vec3{ 0,1,0 });

				forward = glm::normalize(rotation * glm::vec4{ 0,0,1,0 });
			}

			glm::mat4 inv = glm::inverse(viewMatrix);
			right = inv[0];
			up = inv[1];

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

		glm::vec2 GetMouseDelta()
		{
			glm::vec2 oldMousePos{ mousePos };
			double mouseX{}, mouseY{};
			glfwGetCursorPos(m_pWindow->GetWindow(), &mouseX, &mouseY);
			mousePos = { static_cast<float>(mouseX), static_cast<float>(mouseY) };
			return { mousePos - oldMousePos };
		}

		glm::vec2 GetMousePos()
		{
			double mouseX{}, mouseY{};
			glfwGetCursorPos(m_pWindow->GetWindow(), &mouseX, &mouseX);
			return { static_cast<float>(mouseX), static_cast<float>(mouseY) };
		}
	private:

		Window* m_pWindow{};

		glm::vec2 mousePos{};
		glm::vec2 mouseDelta{};
	};
}