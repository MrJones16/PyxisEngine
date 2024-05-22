#pragma once

#include "Pyxis.h"
#include "Pyxis/Game/Scene.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include "Panels/Panel.h"
#include "Panels/ProfilingPanel.h"

#include "World.h"


namespace Pyxis
{
	

	class GameLayer : public Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer() = default;

		virtual void OnAttach();
		virtual void OnDetatch();

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
		bool OnWindowResizeEvent(WindowResizeEvent& event);
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);

		enum BrushType {
			circle = 0, square = 1, end
		};

	private:

		void PaintElementAtCursor();
		//game things
		Ref<World> m_World;
		std::map<std::string, Element> m_ElementsMap;
		std::map<int, std::string> m_IndexToElement;
		//Ref<Chunk> m_Chunk;

		//scene things
		OrthographicCameraController m_OrthographicCameraController;
		glm::vec2 m_ViewportSize;
		Ref<Scene> m_ActiveScene;

		bool m_SceneViewIsFocused = false;

		//Tools / Panels
		std::vector<Ref<Panel>> m_Panels;
		Ref<ProfilingPanel> m_ProfilingPanel;

		//player tools
		int m_SelectedElementIndex = 1;
		float m_BrushSize = 1;
		int m_BrushType = BrushType::circle;

		//testing / game
		bool m_SimulationRunning = false;
	};
}
