#include "pxpch.h"
#include "Application.h"

#include "Input.h"
#include <GLFW/glfw3.h>
#include "Pyxis/Renderer/Renderer.h"

#include <Pyxis/Events/EventSignals.h>

#include <GLFW/glfw3.h>//temp for time

namespace Pyxis 
{

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name, uint32_t width, uint32_t height) {

		PX_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		Pyxis::WindowProps props = WindowProps(name, width, height);
		m_Window = std::unique_ptr<Window>(Window::Create(props));
		m_Window->SetEventCallBack(PX_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		Renderer::Init(m_Window->GetWidth(), m_Window->GetHeight());
		/*if (!Pyxis::Network::Network_Init())
		{
			m_Running = false;
		}*/
		m_ImGuiLayer = CreateRef<ImGuiLayer>();
		PushOverlay(m_ImGuiLayer);
	}

	Application::Application(const std::string& name, bool consoleOnly) {

		PX_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_ImGuiLayer = nullptr;
	}

	Application::~Application() {
		//Pyxis::Network::Network_Shutdown();
	}

	void Application::Sleep(int milliseconds) {

		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	void Application::Close()
	{
		m_Running = false;
		/*while (m_LayerStack.begin() != m_LayerStack.end())
		{
			Layer* layer = *(m_LayerStack.end() - 1);
			m_LayerStack.PopLayer(layer);
			delete layer;
		}*/
		//Sleep(5000);
	}

	void Application::OnEvent(Event& e)
	{
		//PX_CORE_INFO("Event {0}", e);
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(PX_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(PX_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}

		//todo: decide if handled events should be passed?
		switch (e.GetEventType())
		{
		case Pyxis::EventType::None:
			break;
		case Pyxis::EventType::WindowClose:
			break;
		case Pyxis::EventType::WindowResize:
			EventSignal::s_WindowResizeEventSignal(static_cast<WindowResizeEvent&>(e));
			break;
		case Pyxis::EventType::WindowFocus:
			break;
		case Pyxis::EventType::WindowLostFocus:
			break;
		case Pyxis::EventType::WindowMoved:
			break;
		case Pyxis::EventType::AppTick:
			break;
		case Pyxis::EventType::AppUpdate:
			break;
		case Pyxis::EventType::AppRender:
			break;
		case Pyxis::EventType::KeyPressed:
			break;
		case Pyxis::EventType::KeyReleased:
			break;
		case Pyxis::EventType::KeyTyped:
			break;
		case Pyxis::EventType::MouseButtonPressed:
			break;
		case Pyxis::EventType::MouseButtonReleased:
			break;
		case Pyxis::EventType::MouseMoved:
			break;
		case Pyxis::EventType::MouseScrolled:
			break;
		default:
			break;
		}

	}

	void Application::PushLayer(Ref<Layer> layer)
	{
		m_LayersToAdd.push(layer);
		//m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Ref<Layer> layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	/// <summary>
	/// Pops the layer before the next update.
	/// Useful for applications to control the layer stack, as
	/// popping a layer immediately would cause the stack to
	/// be altered duing iteration
	/// </summary>
	void Application::PopLayerQueue(Ref<Layer> layer)
	{
		m_LayersToRemove.push(layer);
	}

	//Use when you need to remove the layer immediately, for use if your layer is not
	//a pointer that can be deleted. 
	void Application::PopLayer(Ref<Layer> layer)
	{
		m_LayerStack.PopLayer(layer);
	}

	void Application::Run() {
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			while (!m_LayersToAdd.empty())
			{
				m_LayerStack.PushLayer(m_LayersToAdd.front());
				m_LayersToAdd.pop();
			}

			while (!m_LayersToRemove.empty())
			{
				m_LayerStack.PopLayer(m_LayersToRemove.front());
				m_LayersToRemove.pop();
			}


			if (!m_Minimized) {
				for (Ref<Layer>& layer : m_LayerStack)
					layer->OnUpdate(timestep);
			}

			m_ImGuiLayer->Begin();
			for (Ref<Layer>& layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();


			m_Window->OnUpdate();
		}


	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		//Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}