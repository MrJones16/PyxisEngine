#pragma once

#include <map>
#include <Pyxis/Nodes/Node.h>
#include <Pyxis/Nodes/RigidBody2D.h>
#include <box2d/b2_world.h>
#include <unordered_set>

namespace Pyxis
{
	/// <summary>
	/// A static wrapper for a B2World system, which would allow for any rigid body object or 
	/// b2body implementation to create itself
	/// 
	/// Nodes must manage the Physics2D themselves, it is not handled by the engine or the scene layer.
	/// So you must use like a physics manager to update the physics. which also allows for more flexibility
	/// for physics updating & use
	/// 
	/// </summary>
	class Physics2D
	{
	private:
		/// <summary>
		/// 
		/// </summary>
		inline static b2World* m_World = nullptr;
	protected:
		//inline static std::map<b2Body*, RigidBody2D*> s_BodyToNode;
		//inline static std::unordered_set<RigidBody2D*> s_RigidBodies;
		//inline static std::queue<RigidBody2D*> s_RigidBodiesToAdd;
		//inline static std::queue<RigidBody2D*> s_RigidBodiesToRemove;

		

	public:
		inline static int32_t m_VelocityIterations = 6;
		inline static int32_t m_PositionIterations = 2;
		inline static float m_Step = 1.0f / 60.0f;

		inline static b2World* GetWorld()
		{
			if (m_World == nullptr)
			{
				m_World = new b2World({ 0, -9.8f });
			}
			else
			{
				return m_World;
			}
		}

		inline static void ResetWorld()
		{			
			PX_TRACE("new b2world made");
			b2World* world = new b2World({ 0, -9.8f });
			std::queue<b2Body*> IterateQueue;
			b2Body* body = m_World->GetBodyList();
			while (body != nullptr)
			{
				RigidBody2D* rb = (RigidBody2D*)(body->GetUserData().pointer);
				body = body->GetNext();
				if (rb != nullptr)
				{
					rb->TransferWorld(world);					
				}
			}
			delete m_World;
			m_World = world;
		}

		inline static void DeleteWorld()
		{
			if (m_World != nullptr)
			{
				b2Body* body = m_World->GetBodyList();
				while (body != nullptr)
				{
					RigidBody2D* rb = (RigidBody2D*)(body->GetUserData().pointer);
					body = body->GetNext();
					if (rb != nullptr)
					{
						rb->DestroyBody();
						rb->QueueFree();
					}

				}

				//now delete the box2d world
				delete m_World;
				m_World = nullptr;
			}
		}


		inline static void Step()
		{
			m_World->Step(m_Step, m_VelocityIterations, m_PositionIterations);
			b2Body* body = m_World->GetBodyList();
			while (body != nullptr)
			{
				RigidBody2D* rb = (RigidBody2D*)(body->GetUserData().pointer);
				body = body->GetNext();
				if (rb != nullptr)
				{
					rb->OnPhysicsUpdate();
				}
			}
		}

		friend class RigidBody2D;
	};

	

}
