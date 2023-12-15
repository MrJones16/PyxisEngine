#include "pxpch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"

#include "Pyxis/Application.h"

//TEMPORARY  shouldnt do this? 
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Pyxis
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
		

	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::SetClipboardText(void* user_data, const char* text)
	{
		glfwSetClipboardString((GLFWwindow*)user_data, text);
	}

	const char* ImGuiLayer::GetClipboardText(void* user_data)
	{
		return glfwGetClipboardString((GLFWwindow*)user_data);
	}

	void ImGuiLayer::OnAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.BackendFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
		io.BackendFlags |= ImGuiConfigFlags_DockingEnable;

		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_LeftShift] = GLFW_KEY_LEFT_SHIFT;
		io.KeyMap[ImGuiKey_RightShift] = GLFW_KEY_LEFT_SHIFT;
		io.KeyMap[ImGuiKey_LeftCtrl] = GLFW_KEY_LEFT_CONTROL;
		io.KeyMap[ImGuiKey_RightCtrl] = GLFW_KEY_RIGHT_CONTROL;
		io.KeyMap[ImGuiKey_W] = GLFW_KEY_W;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_S] = GLFW_KEY_S;
		io.KeyMap[ImGuiKey_D] = GLFW_KEY_D;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;

		//io.SetClipboardTextFn = ImGuiLayer::SetClipboardText;
		//io.GetClipboardTextFn = ImGuiLayer::GetClipboardText;

		ImGui_ImplOpenGL3_Init("#version 460");
	}

	void ImGuiLayer::OnDetach()
	{

	}

	void ImGuiLayer::OnUpdate()
	{

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		float time = (float)glfwGetTime();
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		static bool show = true;
		//ImGui::DockSpaceOverViewport();
		ImGui::ShowDemoWindow(&show);
		ImGui::Begin("Hello!");
		ImGui::Text("yo");
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<MouseButtonPressedEvent>  (BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent> (BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>          (BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>       (BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
		dispatcher.Dispatch<KeyPressedEvent>          (BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>         (BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyTypedEvent>            (BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		//dispatcher.Dispatch<WindowCloseEvent>        (BIND_EVENT_FN(ImGuiLayer::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>        (BIND_EVENT_FN(ImGuiLayer::OnWindowResizeEvent));
		//dispatcher.Dispatch<WindowFocusEvent>        (BIND_EVENT_FN(ImGuiLayer::OnWindowFocusEvent));
		//dispatcher.Dispatch<WindowMoveEvent>         (BIND_EVENT_FN(ImGuiLayer::OnWindowMoveEvent));
		//dispatcher.Dispatch<AppTickEvent>            (BIND_EVENT_FN(ImGuiLayer::OnAppTickEvent));
		//dispatcher.Dispatch<AppUpdateEvent>          (BIND_EVENT_FN(ImGuiLayer::OnAppUpdateEvent));
		//dispatcher.Dispatch<AppRenderEvent>          (BIND_EVENT_FN(ImGuiLayer::OnAppRenderEvent));
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = true;

		return false;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = false;
		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(e.GetX(), e.GetY());

		return false;
	}

	bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH = e.GetXOffset();
		io.MouseWheel = e.GetYOffset();

		return false;
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = true;

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;
		return false;
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int keycode = e.GetKeyCode();
		if (keycode > 0 && keycode < 0x10000)
			io.AddInputCharacter((unsigned short)keycode);

		return false;
	}

	bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		glViewport(0, 0, e.GetWidth(), e.GetHeight());

		return false;
	}

}