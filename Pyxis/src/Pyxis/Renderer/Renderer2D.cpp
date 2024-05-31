#include "pxpch.h"
#include "Renderer2D.h"
#include "RenderCommand.h"

#include "VertexArray.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Pyxis
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	struct RendererData2D
	{
		//QUADS
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
		glm::vec4 QuadVertexPositions[4] = {
			{-0.5f, -0.5f, 0, 1},
			{0.5f, -0.5f, 0, 1},
			{0.5f, 0.5f, 0, 1},
			{-0.5f, 0.5f, 0, 1}
		};

		//idk why ref of texture2d is better here, i guess for
		//if a texture comes from something not a texture2D?
		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotsIndex = 1;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		
		Renderer2D::Statistics Stats;
	};

	

	static RendererData2D s_Data; // can make this a pointer

	void Renderer2D::Init()
	{
		//initialize the renderer2d primitive things
		//s_Data = new RendererData2D();


		s_Data.QuadVertexArray = VertexArray::Create();
		//m_OrthographicCameraController = Pyxis::OrthographicCameraController(5, 9 / 16, -100, 100);
		s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data.TextureShader->Bind();
		int samplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
		s_Data.TextureShader->SetFloat("u_TilingFactor", 1);


		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexID"},
			{ShaderDataType::Float, "a_TilingFactor"},
		};
		s_Data.QuadVertexBuffer->SetLayout(layout);
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];


		uint32_t* QuadIndices = new uint32_t[s_Data.MaxIndices];
		//set indices
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i+= 6)
		{
			QuadIndices[i + 0] = offset + 0;
			QuadIndices[i + 1] = offset + 1;
			QuadIndices[i + 2] = offset + 2;

			QuadIndices[i + 3] = offset + 2;
			QuadIndices[i + 4] = offset + 3;
			QuadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> QuadIndexBuffer = IndexBuffer::Create(QuadIndices, s_Data.MaxIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(QuadIndexBuffer);
		delete[] QuadIndices;


		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t WhiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&WhiteTextureData, sizeof(WhiteTextureData));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		//frame buffer
		//m_FrameBuffer = FrameBuffer::Create(1920, 1080);
		//Renderer::AddFrameBuffer(m_FrameBuffer);
	}

	void Renderer2D::Shutdown()
	{
		//delete s_Data;
	}

	void Renderer2D::BeginScene(const Pyxis::OrthographicCamera& camera)
	{	
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.QuadIndexCount = 0;

		s_Data.TextureSlotsIndex = 1;
	}
	
	void Renderer2D::EndScene()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		//send data to buffer
		uint32_t size = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, size);

		for (uint32_t i = 0; i < s_Data.TextureSlotsIndex; i++)
		{
			s_Data.TextureSlots[i]->Bind(i);
		}

		//draw
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.QuadIndexCount = 0;

		s_Data.TextureSlotsIndex = 1;
#if STATISTICS
		s_Data.Stats.DrawCalls++;
#endif
	}

	/// <summary>
	/// Submit a transform and color to be drawn
	/// </summary>
	void Renderer2D::DrawQuad(glm::mat4 transform, const glm::vec4& color, float tilingFactor)
	{
		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		//texture
		float TexIndex = 0.0f; // white texture

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	/// <summary>
	/// Submit a transform and texture to be drawn
	/// </summary>
	void Renderer2D::DrawQuad(glm::mat4 transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
	}
		if (TexIndex == 0.0f)
		{
			if (s_Data.TextureSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.TextureSlotsIndex;
			s_Data.TextureSlots[s_Data.TextureSlotsIndex] = texture;
			s_Data.TextureSlotsIndex = s_Data.TextureSlotsIndex + 1;
		}

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	/// <summary>
	/// Submit a transform and sub-texture to be drawn
	/// </summary>
	void Renderer2D::DrawQuad(glm::mat4 transform, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& tintColor)
	{
		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2* textureCoords = subTexture->GetTexCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
		}
		if (TexIndex == 0.0f)
		{
			if (s_Data.TextureSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.TextureSlotsIndex;
			s_Data.TextureSlots[s_Data.TextureSlotsIndex] = texture;
			s_Data.TextureSlotsIndex = s_Data.TextureSlotsIndex + 1;
		}

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}


	void Renderer2D::DrawQuad(const glm::vec2 position, const glm::vec2& size, const glm::vec4& color, float tilingFactor)
	{
		DrawQuad({ position.x, position.y, 0 }, size, color, tilingFactor);
	}

	void Renderer2D::DrawQuad(const glm::vec3 position, const glm::vec2& size, const glm::vec4& color, float tilingFactor)
	{
		//check if we need to flush
		if (s_Data.TextureSlotsIndex == s_Data.MaxTextureSlots)
		{
			Flush();
		}

		//texture coords
		const glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		
		//texture
		float TexIndex = 0.0f; // white texture

		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, { size.x, size.y, 0 });

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			//heavy math so hurts on debug
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif

	}

	void Renderer2D::DrawQuad(const glm::vec2 position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0 }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3 position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{

		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
		}
		if (TexIndex == 0.0f)
		{
			if (s_Data.TextureSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.TextureSlotsIndex;
			s_Data.TextureSlots[s_Data.TextureSlotsIndex] = texture;
			s_Data.TextureSlotsIndex = s_Data.TextureSlotsIndex + 1;
		}
		
		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, { size.x, size.y, 0 });

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawQuad(const glm::vec2 position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0 }, size, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3 position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& tintColor)
	{

		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2* textureCoords = subTexture->GetTexCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
		}
		if (TexIndex == 0.0f)
		{
			if (s_Data.TextureSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.TextureSlotsIndex;
			s_Data.TextureSlots[s_Data.TextureSlotsIndex] = texture;
			s_Data.TextureSlotsIndex = s_Data.TextureSlotsIndex + 1;
		}
		

		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, { size.x, size.y, 0 });

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2 position, const glm::vec2& size, float rotation, const glm::vec4& color, float tilingFactor)
	{
		DrawRotatedQuad({ position.x, position.y, 0 }, size, rotation, color, tilingFactor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3 position, const glm::vec2& size, float rotation, const glm::vec4& color, float tilingFactor)
	{
		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		//Texture
		float TexIndex = 0.0f; // white texture

		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, -rotation, { 0,0,1 });
		transform = glm::scale(transform, { size.x, size.y, 0 });

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2 position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0 }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3 position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
		}
		if (TexIndex == 0.0f)
		{
			if (s_Data.TextureSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.TextureSlotsIndex;
			s_Data.TextureSlots[s_Data.TextureSlotsIndex] = texture;
			s_Data.TextureSlotsIndex = s_Data.TextureSlotsIndex + 1;
		}

		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, -rotation, { 0,0,1 });
		transform = glm::scale(transform, { size.x, size.y, 0 });

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2 position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0 }, size, rotation, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3 position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& tintColor)
	{
		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2* textureCoords = subTexture->GetTexCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
		}
		if (TexIndex == 0.0f)
		{
			if (s_Data.TextureSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.TextureSlotsIndex;
			s_Data.TextureSlots[s_Data.TextureSlotsIndex] = texture;
			s_Data.TextureSlotsIndex = s_Data.TextureSlotsIndex + 1;
		}

		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, -rotation, { 0,0,1 });
		transform = glm::scale(transform, { size.x, size.y, 0 });

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = TexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}
	
}