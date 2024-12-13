#pragma once

#include "GameLayer.h"



namespace Pyxis
{

	class SingleplayerGameLayer : public GameLayer, std::enable_shared_from_this<SingleplayerGameLayer>
	{
	public:
		SingleplayerGameLayer();
		virtual ~SingleplayerGameLayer();

		//////////////////////////////////////
		/// Game Functions
		//////////////////////////////////////
		void Start();

		//////////////////////////////////////
		/// Layer Functions
		//////////////////////////////////////
		//virtual void OnAttach() override;
		//virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		//virtual void OnEvent(Event& e) override;

		


	public:

	private:

	};
}
