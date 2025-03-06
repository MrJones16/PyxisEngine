#include "ElementParticle.h"


namespace Pyxis
{	


	ElementParticle::ElementParticle(const glm::vec2& position, const glm::vec2& velocity, const Element& element) 
		: m_Position(position), m_Velocity(velocity), m_Element(element)
	{

	}

	void ElementParticle::Update()
	{
		m_Velocity.y -= 0.25f;
	}

	void ElementParticle::Render()
	{
		//Render the particle
		glm::vec3 position = { m_Position.x / CHUNKSIZEF, m_Position.y / CHUNKSIZEF, 0.1f };

		float r = float(m_Element.m_Color & 0x000000FF) / 255.0f;
		float g = float((m_Element.m_Color & 0x0000FF00) >> 8) / 255.0f;
		float b = float((m_Element.m_Color & 0x00FF0000) >> 16) / 255.0f;
		float a = float((m_Element.m_Color & 0xFF000000) >> 24) / 255.0f;
		glm::vec4 vecColor = glm::vec4(r, g, b, a);

		float halfSize = PIXELSIZE / 2.0f;
		Renderer2D::DrawQuad(position + glm::vec3(halfSize, halfSize, 0), glm::vec2(PIXELSIZE, PIXELSIZE), vecColor);
	}
}
