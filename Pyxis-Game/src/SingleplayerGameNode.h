#pragma once

#include "GameNode.h"


namespace Pyxis
{

	class SinglePlayerGameNode : public GameNode
	{
	public:
		SinglePlayerGameNode() 
		{
			//scene heirarchy done in constructor since we have no scene serialization
		};
		virtual ~SinglePlayerGameNode() {};

		//////////////////////////////////////
		/// Game Functions
		//////////////////////////////////////
		void Start()
		{
			for (int x = -1; x <= 1; x++)
			{
				for (int y = -1; y <= 1; y++)
				{
					m_World.AddChunk({ x,y });
					m_World.GenerateChunk(m_World.GetChunk({ x,y }));
				}
			}
			
		}


		virtual void OnUpdate(Timestep ts) override
		{
			PROFILE_SCOPE("GameLayer::OnUpdate");

			GameUpdate(ts);
		}

		virtual void OnRender()
		{
			
		}

		virtual void OnFixedUpdate() override
		{
			PROFILE_SCOPE("Simulation Update");

			//for singleplayer, just construct your own merged tick and 
			//handle it
			MergedTickClosure tc;
			tc.AddTickClosure(m_CurrentTickClosure, 0);
			HandleTickClosure(tc);

			//reset tick closure
			m_CurrentTickClosure = TickClosure();
		}

		virtual void OnImGuiRender() override
		{
			//ClientImGuiRender();
		}

	};
}
