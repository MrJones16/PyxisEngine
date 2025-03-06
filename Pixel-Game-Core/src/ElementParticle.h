#pragma once

#include "Element.h"
#include <Pyxis/Renderer/Renderer2D.h>

namespace Pyxis
{

	// A particle is an element that has a velocity & position, and generally isn't in the world until it hits something.		
	class ElementParticle
	{
	public:
		glm::vec2 m_Position = { 0,0 };
		glm::vec2 m_Velocity = { 0,0 };
		Element m_Element;
		ElementTypeType m_CollisionFlags = static_cast<ElementTypeType>(ElementType::solid) | static_cast<ElementTypeType>(ElementType::movableSolid) | static_cast<ElementTypeType>(ElementType::liquid);
	public:
		ElementParticle(const glm::vec2& position, const glm::vec2& velocity, const Element& element);			

		//update the velocity or any other property of a particle.
		//When the world updates this particle, it will call this function before moving the particle.
		//this function should not move the particle using its velocity, as the world does that to check for
		//collisions to stop the particle.
		virtual void Update();

		void Render();
		
	};

}