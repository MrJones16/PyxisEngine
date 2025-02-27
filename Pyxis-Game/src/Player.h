#pragma once
#include "PixelBody2D.h"
#include <Pyxis/Events/EventSignals.h>

#include <Pyxis/Nodes/CameraNode.h>

namespace Pyxis
{
	class Player : public PixelBody2D
	{
	public:

		Reciever<void(KeyPressedEvent&)> m_KeyPressedReciever;
		

		Player(const std::string& name, b2BodyType type, World* world, std::vector<PixelBodyElement>& elements, bool CreatedFromSplit = false)
			: PixelBody2D(name, type, world, elements, CreatedFromSplit), m_KeyPressedReciever(this, &Player::OnKeyPressedEvent)
		{
			EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);
			m_B2BodyDef.fixedRotation = true;
			m_B2Body->SetFixedRotation(true);
		}

		Player(UUID id) : PixelBody2D(id),
			m_KeyPressedReciever(this, &Player::OnKeyPressedEvent)
		{
			EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);
			m_B2BodyDef.fixedRotation = true;
		}

		~Player()
		{

		}

		void Serialize(json& j) override
		{
			PixelBody2D::Serialize(j);
			j["Type"] = "Player";
		}

		void Deserialize(json& j) override
		{
			PixelBody2D::Deserialize(j);
		}

		void OnKeyPressedEvent(KeyPressedEvent& e)
		{
			PX_TRACE("Key Pressed: {0}", e.GetKeyCode());
			if (e.GetKeyCode() == PX_KEY_W)
			{
				PX_TRACE("Space Pressed");
				m_B2Body->ApplyLinearImpulseToCenter({ 0, 0.3f }, true);
			}

			//Save to file this player, so we can debug test easier by loading it
			if (e.GetKeyCode() == PX_KEY_J)
			{
				//open a file and write to it the json
				std::ofstream file("assets/Player.json");
				json j;
				Serialize(j);
				file << j.dump(4);
				file.close();
			}
			
		}

		void OnPhysicsUpdate() override
		{
			PixelBody2D::OnPhysicsUpdate();

			float maxVelocity = 2;
			if (Input::IsKeyPressed(PX_KEY_A))
			{
				if (m_B2Body->GetLinearVelocity().x > -maxVelocity)
				{
					m_B2Body->ApplyForceToCenter({ -1, 0 }, true);
				}
			}
			if (Input::IsKeyPressed(PX_KEY_D))
			{
				if (m_B2Body->GetLinearVelocity().x < maxVelocity)
				{
					m_B2Body->ApplyForceToCenter({ 1, 0 }, true);
				}
			}


		}

		void OnUpdate(Timestep ts) override
		{
			PixelBody2D::OnUpdate(ts);

			//Camera follow
			CameraNode* camera = dynamic_cast<CameraNode*>(CameraNode::Main());
			camera->SetPosition({ GetPosition().x, GetPosition().y, 0 });
		}

		/*void OnFixedUpdate() override
		{
			PixelBody2D::OnFixedUpdate();			
		}*/

		/*void OnInspectorRender() override
		{
			PixelBody2D::OnInspectorRender();
		}*/

		/*void OnRender() override
		{
			PixelBody2D::OnRender();
		}*/

	};
	REGISTER_SERIALIZABLE_NODE(Player);
}
