#pragma once

#include "Core.h"

#include "Window.h"
#include "Pyxis/Core/LayerStack.h"
#include "Pyxis/Events/Event.h"
#include "Pyxis/Events/ApplicationEvent.h"

#include "Pyxis/ImGui/ImGuiLayer.h"

#include "Pyxis/Renderer/Shader.h"
#include "Pyxis/Renderer/Buffer.h"
#include "Pyxis/Renderer/VertexArray.h"

#include "Pyxis/Core/Timestep.h"

namespace Pyxis
{

	class PYXIS_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent &e);

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	//define in CLIENT
	Application* CreateApplication();
}

