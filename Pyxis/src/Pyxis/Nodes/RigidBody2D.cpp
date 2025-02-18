#include "RigidBody2D.h"
#include <Pyxis/Game/Physics2D.h>

namespace Pyxis
{
	RigidBody2D::RigidBody2D(const std::string& name, b2BodyType type) : Node2D(name)
	{
		m_B2BodyDef.position = { 0,0 };
		m_B2BodyDef.type = type; 
		m_B2BodyDef.userData.pointer = (uintptr_t)this;
		CreateBody(Physics2D::GetWorld());
	}

	RigidBody2D::RigidBody2D(UUID id) : Node2D(id)
	{
		m_HasBody = false;
	}

	RigidBody2D::~RigidBody2D()
	{
		DestroyBody();
	}

	void RigidBody2D::OnPhysicsUpdate()
	{

	}

	void RigidBody2D::OnInspectorRender()
	{
		Node2D::OnInspectorRender();
		int bodyType = GetType();
		ImGui::Text("Type: ");
		{
			ImGui::RadioButton("Static", &bodyType, 0); ImGui::SameLine();
			ImGui::RadioButton("Kinematic", &bodyType, 1); ImGui::SameLine();
			ImGui::RadioButton("Dynamic", &bodyType, 2);
		}
		SetType((b2BodyType)bodyType);
	}


	//TODO: Completely serialize b2bodies, and not just what i am using!
	void RigidBody2D::Serialize(json& j)
	{
		//update the member vars with b2body and then serialize them
		m_Position = GetPosition();
		m_Rotation = GetRotation();

		Node2D::Serialize(j);
		j["Type"] = "RigidBody2D"; // Override type identifier

		//Serialize member variables
		//j["m_HasBody"] = m_HasBody;// this will be determined by if it has the "m_b2Body"
		j["m_CategoryBits"] = m_CategoryBits;
		j["m_MaskBits"] = m_MaskBits;

		//Serialize B2 Body Definition
		//we will re-construct the user data, and 
		j["m_B2BodyDef"]["position"] = m_B2BodyDef.position;
		j["m_B2BodyDef"]["type"] = (int)m_B2BodyDef.type;

		//Serialize B2 Body data
		if (m_HasBody)
		{
			//note: we already have position and angle

			j["m_B2Body"]["linearVelocity"] = m_B2Body->GetLinearVelocity();
			j["m_B2Body"]["linearDamping"] = m_B2Body->GetLinearDamping();

			j["m_B2Body"]["angularVelocity"] = m_B2Body->GetAngularVelocity();
			j["m_B2Body"]["angularDamping"] = m_B2Body->GetAngularDamping();
		}
		
	}

	void RigidBody2D::Deserialize(json& j)
	{
		Node2D::Deserialize(j);

		if (j.contains("m_CategoryBits")) j.at("m_CategoryBits").get_to(m_CategoryBits);
		if (j.contains("m_MaskBits")) j.at("m_MaskBits").get_to(m_MaskBits);

		if (j.contains("m_B2BodyDef"))
		{
			if (j["m_B2BodyDef"].contains("position")) j["m_B2BodyDef"].at("position").get_to(m_B2BodyDef.position);
			if (j["m_B2BodyDef"].contains("type")) j["m_B2BodyDef"].at("type").get_to(m_B2BodyDef.type);
		}

		if (j.contains("m_B2Body"))
		{
			CreateBody(Physics2D::GetWorld());
			
			b2Vec2 linearVelocity = {0,0};
			float linearDamping = 0, angularVelocity = 0, angularDamping = 0;
			if (j["m_B2Body"].contains("linearVelocity")) j["m_B2Body"].at("linearVelocity").get_to(linearVelocity);
			if (j["m_B2Body"].contains("linearDamping")) j["m_B2Body"].at("linearDamping").get_to(linearDamping);
			if (j["m_B2Body"].contains("angularVelocity")) j["m_B2Body"].at("angularVelocity").get_to(angularVelocity);
			if (j["m_B2Body"].contains("angularDamping")) j["m_B2Body"].at("angularDamping").get_to(angularDamping);
			m_B2Body->SetTransform(b2Vec2(m_Position.x, m_Position.y), m_Rotation);
			m_B2Body->SetLinearVelocity(linearVelocity);
			m_B2Body->SetLinearDamping(linearDamping);
			m_B2Body->SetAngularVelocity(angularVelocity);
			m_B2Body->SetAngularDamping(angularDamping);
		}		
	}

	glm::mat4 RigidBody2D::GetWorldTransform()
	{
		auto pos = m_B2Body->GetPosition();
		glm::mat4 localTransform = glm::translate(glm::mat4(), glm::vec3(pos.x, pos.y, m_Layer));
		localTransform = glm::rotate(localTransform, m_Rotation, { 0,0,-1 });
		if (Node2D* parent2D = dynamic_cast<Node2D*>(m_Parent))
		{
			return parent2D->GetWorldTransform() * localTransform;
		}
		else
		{
			return localTransform;
		}
	}

	void RigidBody2D::Translate(const glm::vec2& translation)
	{
		m_Position += translation;
		m_B2Body->SetTransform({ m_Position.x, m_Position.y }, m_Rotation);
	}
	void RigidBody2D::SetPosition(const glm::vec2& position)
	{
		m_Position = position;
		m_B2Body->SetTransform({ m_Position.x, m_Position.y }, m_Rotation);
	}
	glm::vec2 RigidBody2D::GetPosition()
	{
		b2Vec2 vec = m_B2Body->GetPosition();
		return glm::vec2(vec.x, vec.y);
	}

	void RigidBody2D::Rotate(const float radians)
	{
		m_Rotation += radians;
		m_B2Body->SetTransform({ m_Position.x, m_Position.y }, m_Rotation);
	}
	void RigidBody2D::SetRotation(const float radians)
	{
		m_Rotation = radians;
		m_B2Body->SetTransform({ m_Position.x, m_Position.y }, m_Rotation);
	}
	float RigidBody2D::GetRotation()
	{
		return m_B2Body->GetAngle();
	}

	void RigidBody2D::TransferWorld(b2World* world)
	{
		if (m_HasBody)
		{
			//store data from body, then make a new one, then put the new data in.
			B2BodyStorage storage(m_B2Body);
			//trick ourselves to make a new body even though we have one
			m_HasBody = false;
			CreateBody(world);
			storage.TransferData(m_B2Body);
		}
	}

	void RigidBody2D::CreateBody(b2World* world)
	{
		if (!m_HasBody)
		{
			m_HasBody = true;
			m_B2World = world;
			m_B2Body = m_B2World->CreateBody(&m_B2BodyDef);
			//Physics2D::s_RigidBodiesToAdd.push(this);
		}
	}

	void RigidBody2D::DestroyBody()
	{
		if (m_HasBody)
		{
			m_HasBody = false;
			//Physics2D::s_BodyToNode[m_B2Body] = nullptr;
			m_B2World->DestroyBody(m_B2Body);
			m_B2Body = nullptr;
			m_B2World = nullptr;

		}
	}

	void RigidBody2D::SetType(b2BodyType type)
	{
		m_B2BodyDef.type = type;
		m_B2Body->SetType(type);
	}

	b2BodyType RigidBody2D::GetType()
	{
		return m_B2Body->GetType();
	}

	void RigidBody2D::ApplyForce(const glm::vec2& force, const glm::vec2& point)
	{
		m_B2Body->ApplyForce({ force.x, force.y }, { point.x, point.y }, true);
	}

	void RigidBody2D::ApplyForceToCenter(const glm::vec2& force)
	{
		m_B2Body->ApplyForceToCenter({ force.x, force.y }, true);
	}

	void RigidBody2D::AddLinearVelocity(const glm::vec2& velocity)
	{
		m_B2Body->SetLinearVelocity(m_B2Body->GetLinearVelocity() + b2Vec2(velocity.x, velocity.y));
	}

	void RigidBody2D::SetLinearVelocity(const glm::vec2& velocity)
	{
		m_B2Body->SetLinearVelocity({ velocity.x, velocity.y });
	}

	glm::vec2 RigidBody2D::GetLinearVelocity()
	{
		auto velocity = m_B2Body->GetLinearVelocity();
		return glm::vec2(velocity.x, velocity.y);
	}

	void RigidBody2D::AddAngularVelocity(float angVel)
	{
		m_B2Body->ApplyAngularImpulse(angVel, true);
	}

	void RigidBody2D::SetAngularVelocity(float angVel)
	{
		m_B2Body->SetAngularVelocity(angVel);
	}

	float RigidBody2D::GetAngularVelocity()
	{
		return m_B2Body->GetAngularVelocity();
	}

	void RigidBody2D::SetAngularDampening(float damping)
	{
		m_B2Body->SetAngularDamping(damping);
	}

	float RigidBody2D::GetAngularDamping()
	{
		return m_B2Body->GetAngularDamping();
	}

}
