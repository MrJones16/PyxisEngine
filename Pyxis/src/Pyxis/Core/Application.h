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

#include <queue>

namespace Pyxis
{

	class PYXIS_API Application
	{
	public:
		Application(const std::string& name = "Pyxis-Engine", uint32_t width = 1280, uint32_t height = 720);
		Application(const std::string& name, bool consoleOnly);
		virtual ~Application();

		void Close();

		void Sleep(int milliseconds = 1000);

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void PopLayerQueue(Layer* layer);
		void PopLayer(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent &e);

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		std::queue<Layer*> m_LayersToAdd;
		std::queue<Layer*> m_LayersToRemove;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	//define in CLIENT
	Application* CreateApplication();
}

