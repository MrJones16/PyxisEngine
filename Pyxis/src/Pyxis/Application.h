#pragma once

#include "Core.h"

#include "Window.h"
#include "Pyxis/LayerStack.h"
#include "Pyxis/Events/Event.h"
#include "Pyxis/Events/ApplicationEvent.h"

#include "Pyxis/ImGui/ImGuiLayer.h"

#include "Pyxis/Renderer/Shader.h"
#include "Pyxis/Renderer/Buffer.h"

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

		unsigned int m_VertexArray, m_VertexBufferTest, mIndexBufferTest;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
	private:
		static Application* s_Instance;
	};

	//define in CLIENT
	Application* CreateApplication();
}

