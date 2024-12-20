#include "SingleplayerGameLayer.h"

namespace Pyxis
{

	SingleplayerGameLayer::SingleplayerGameLayer()
	{
		
	}

	SingleplayerGameLayer::~SingleplayerGameLayer()
	{

	}

	void SingleplayerGameLayer::Start()
	{
		CreateWorld();
	}

	void SingleplayerGameLayer::OnUpdate(Timestep ts)
	{
		PROFILE_SCOPE("GameLayer::OnUpdate");
		m_OrthographicCameraController.OnUpdate(ts);
		m_Scene->Update(ts);

		//rendering
		#if STATISTICS
		Renderer2D::ResetStats();
		#endif

		{
			PROFILE_SCOPE("Renderer Prep");
			m_SceneFrameBuffer->Bind();
			RenderCommand::SetClearColor({ 198 / 255.0f, 239 / 255.0f, 249 / 255.0f, 1 });
			RenderCommand::Clear();
			Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
		}

		
		//only run per tick rate
		auto time = std::chrono::high_resolution_clock::now();
		if (m_TickRate > 0 &&
			std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count()
			-
			std::chrono::time_point_cast<std::chrono::microseconds>(m_UpdateTime).time_since_epoch().count()
			>= (1.0f / m_TickRate) * 1000000.0f)
		{
			PROFILE_SCOPE("Simulation Update");

			m_UpdateTime = time;
			//for singleplayer, just construct your own merged tick and 
			//handle it
			MergedTickClosure tc;
			tc.AddTickClosure(m_CurrentTickClosure, 0);
			HandleTickClosure(tc);

			//reset tick closure
			m_CurrentTickClosure = TickClosure();
		}

		GameUpdate(ts);

		m_Scene->Render();

		Renderer2D::EndScene();

		m_SceneFrameBuffer->Unbind();
	}

	void SingleplayerGameLayer::OnImGuiRender()
	{
		auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MainDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
		ClientImGuiRender(dock);
	}

	

}