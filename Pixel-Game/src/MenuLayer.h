#pragma once

#include "GameLayer.h"


namespace Pyxis
{
	class MenuLayer : public Layer
	{
	public:
		MenuLayer();
		virtual ~MenuLayer();

		//Layer functions
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		void AttachGameLayer();
		void DetachGameLayer();

		void FailedToConnect();
		//bool OnWindowResizeEvent(WindowResizeEvent& event);
		//bool OnKeyPressedEvent(KeyPressedEvent& event);
		//bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		//bool OnMouseScrolledEvent(MouseScrolledEvent& event);

		//std::pair<float, float> GetMousePositionScene();

	private:

		GameLayer* m_GameLayer = new GameLayer();
		bool m_GameLayerAttached = false;

		//UI things
		bool m_InMainMenu = true;
		char m_InputAddress[22] = "127.0.0.1:21218";

	};
}
