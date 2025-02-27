#include "pxpch.h"
#include "Pyxis/Core/Input.h"

#include "Pyxis/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Pyxis
{

	bool Input::IsKeyPressed(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode); 
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}
	glm::ivec2 Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { (float)xpos, (float)ypos};
	}

	int Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	int Input::GetMouseY()
	{
		return GetMousePosition().y;
	}
}