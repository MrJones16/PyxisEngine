#pragma once

#include <Pyxis/Nodes/Node2D.h>

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <poly2tri.h>
#include <box2d/b2_settings.h>


//override json conversion for glm objects

// Serialize b2Vec2
inline void to_json(json& j, const b2Vec2& vec) {
	j = json{ {"x", vec.x}, {"y", vec.y} };
}
// Deserialize glm::vec2
inline void from_json(const json& j, b2Vec2& vec) {
	vec.x = j.at("x").get<float>();
	vec.y = j.at("y").get<float>();
}



namespace Pyxis
{
	//UUID uuid, const glm::ivec2& size, std::unordered_map<glm::ivec2, RigidBodyElement, HashVector> elements, b2BodyType type, b2World* world)
		
	struct B2BodyStorage
	{
	public:
		b2Vec2 position;
		b2Vec2 linearVelocity;
		float linearDamping;
		float angle;
		float angularVelocity;
		float angularDamping;

		B2BodyStorage(b2Body* body)
		{
			position		= body->GetPosition();
			linearVelocity	= body->GetLinearVelocity();
			linearDamping	= body->GetLinearDamping();
			angle			= body->GetAngle();
			angularVelocity = body->GetAngularVelocity();
			angularDamping	= body->GetAngularDamping();
		}

		void TransferData(b2Body* body)
		{
			body->SetTransform(position, angle);
			body->SetLinearVelocity(linearVelocity);
			body->SetLinearDamping(linearDamping);
			body->SetAngularVelocity(angularVelocity);
			body->SetAngularDamping(angularDamping);
		}
	};

	/// <summary>
	/// A B2BodyNode is a node that tries to correlate the b2 transform with the node's transform, and functions like a standard rigid body.
	/// </summary>
	class RigidBody2D : public Node2D
	{
	protected:
		b2Body* m_B2Body = nullptr;
		b2World* m_B2World = nullptr;
		bool m_HasBody = false;

		//todo: make functions to use these!
		uint16_t m_CategoryBits = 1;
		uint16_t m_MaskBits = 0xFFFF;
		
		b2BodyDef m_B2BodyDef;
		

	public:
		RigidBody2D(const std::string& name, b2BodyType type);
		RigidBody2D(UUID id);
		~RigidBody2D();

		/// <summary>
		/// this will be called by Physics2D::step
		/// </summary>
		virtual void OnPhysicsUpdate();

		virtual void OnInspectorRender() override;


		//Serialization
		void Serialize(json& j) override;
		
		void Deserialize(json& j) override;
		

		////////////////////////////////////
		///   Overrides for 2D Transform
		////////////////////////////////////
		virtual glm::mat4 GetWorldTransform();		

		virtual void Translate(const glm::vec2& translation) override;		
		virtual void SetPosition(const glm::vec2& position) override;		
		virtual glm::vec2 GetPosition();		

		virtual void Rotate(const float radians) override;		
		virtual void SetRotation(const float radians) override;		
		virtual float GetRotation();
				

		///////////////////////////////////////////////
		///   Functions for rigid bodies / B2Bodies
		///////////////////////////////////////////////

		/// <summary>
		/// needed for resetting the world without destroying the game object!
		/// 
		/// To be implemented correctly, it needs to not destroy the previous
		/// b2body, and just create a new one in the new world. the old world
		/// will be deleted and that will destroy the old b2bodies.
		/// </summary>
		/// <param name="world"></param>
		virtual void TransferWorld(b2World* world);

		//creates the underlying b2body. might be better to switch this to take the
		//world as parameter
		virtual void CreateBody(b2World* world);

		/// <summary>
		/// Frees the underlying b2body and resets the world.
		/// You need to be able to save the transform
		/// </summary>
		virtual void DestroyBody();

		void ClearFixtures();

		void SetType(b2BodyType type);		
		b2BodyType GetType();		

		//Forces
		void ApplyForce(const glm::vec2& force, const glm::vec2& point);		
		void ApplyForceToCenter(const glm::vec2& force);
				
		///Linear Velocity
		void AddLinearVelocity(const glm::vec2& velocity);		
		void SetLinearVelocity(const glm::vec2& velocity);		
		glm::vec2 GetLinearVelocity();
		
		//Angular Velocity
		void AddAngularVelocity(float angVel);	
		void SetAngularVelocity(float angVel);
		float GetAngularVelocity();		

		//Angular Dampening
		void SetAngularDampening(float damping);
		float GetAngularDamping();
		

		/* What needs to be made
		bool CreateB2Body(b2World* world);
		std::vector<PixelRigidBody*> RecreateB2Body(unsigned int randSeed, b2World* world);

		void SetPixelPosition(const glm::ivec2& position);
		void SetTransform(const glm::vec2& position, float rotation);
		void SetPosition(const glm::vec2& position);
		void SetRotation(float rotation);
		void SetAngularVelocity(float velocity);
		void SetLinearVelocity(const b2Vec2& velocity);

		glm::ivec2 GetPixelPosition();
		*/
		
		/* overridable
		virtual void ResetLocalTransform();
		virtual void SetLocalTransform(const glm::mat4& transform);
		glm::mat4& GetLocalTransform();
		virtual void Translate(glm::vec3 translation);
		virtual void Rotate(glm::vec3 rotation);
		virtual void SetScale(glm::vec3 scale);
		virtual void Scale(glm::vec3 scale);
		*/
	};

	REGISTER_SERIALIZABLE_NODE(RigidBody2D);
}