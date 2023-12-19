#pragma once

#include "Core.h"

#include "Window.h"
#include "Pyxis/LayerStack.h"
#include "Pyxis/Events/Event.h"
#include "Pyxis/Events/ApplicationEvent.h"

#include "Pyxis/ImGui/ImGuiLayer.h"

#include "Pyxis/Renderer/Shader.h"
#include "Pyxis/Renderer/Buffer.h"
#include "Pyxis/Renderer/VertexArray.h"

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
		bool OnWindowClose(WindowCloseEvent &e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;

		std::shared_ptr<Shader> m_OrangeShader;
		std::shared_ptr<VertexArray> m_SquareVA;
	private:
		static Application* s_Instance;
	};

	//define in CLIENT
	Application* CreateApplication();
}

