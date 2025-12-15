#include "ElementParticle.h"


namespace Pyxis
{	


	ElementParticle::ElementParticle(const glm::vec2& position, const glm::vec2& velocity, const Element& element) 
		: m_Position(position), m_Velocity(velocity), m_Element(element)
	{

	}

	void ElementParticle::Update()
	{
		//m_Velocity.y -= 0.25f;
	}

	void ElementParticle::Render()
	{
		//dont render very slow particles
		if (m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y < DEADSPEED) return;

		//Render the particle
		glm::vec3 position = { m_Position.x, m_Position.y, 0.1f };

		float r = float(m_Element.m_Color & 0x000000FF) / 255.0f;
		float g = float((m_Element.m_Color & 0x0000FF00) >> 8) / 255.0f;
		float b = float((m_Element.m_Color & 0x00FF0000) >> 16) / 255.0f;
		float a = float((m_Element.m_Color & 0xFF000000) >> 24) / 255.0f;
		glm::vec4 vecColor = glm::vec4(r, g, b, a);
		//if (m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y < DEADSPEED) vecColor = {1,0,0,1};

		
		Renderer2D::DrawQuad(position + glm::vec3(0.5f, 0.5f, 0), glm::vec2(1, 1), vecColor);
	}
}
