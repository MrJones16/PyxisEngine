#pragma once

#include <Pyxis/Core/Layer.h>
#include <Pyxis/Renderer/Camera.h>
#include <Pyxis/Renderer/FrameBuffer.h>

#include <Pyxis/Nodes/Node.h>
#include <Pyxis/Nodes/UI.h>
#include <Pyxis/Nodes/CameraNode.h>

#include <Pyxis/Events/EventSignals.h>


namespace Pyxis
{
	class SceneLayer : public Layer
	{
	public:
		SceneLayer(bool debug = false);
		virtual ~SceneLayer();

		//Layer functions
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;


		

		glm::ivec2 GetMousePositionImGui();

		bool OnWindowResizeEvent(WindowResizeEvent& event);
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event);
		bool OnMouseScrolledEvent(MouseScrolledEvent& event);

		//std::pair<float, float> GetMousePositionScene();

	public:
		//scene
		Node m_RootNode;
		Ref<Camera> m_MainCamera;

	private:

		//debug heirarchy / inspector
		bool m_Debug = false;
		virtual void DrawNodeTree(Ref<Node> Node);
		Ref<Node> m_SelectedNode;


		//viewport
		Ref<FrameBuffer> m_SceneFrameBuffer;
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];


		//fixed update
		double m_FixedUpdateRate = 60.0f;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_FixedUpdateTime = std::chrono::high_resolution_clock::now();

		//Other

		uint32_t m_HoveredNodeID = 0;

	};
}
