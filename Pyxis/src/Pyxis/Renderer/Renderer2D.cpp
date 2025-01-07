#include "pxpch.h"
#include "Renderer2D.h"
#include "RenderCommand.h"

#include "VertexArray.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>

namespace Pyxis
{

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		uint32_t NodeID;
	};

	struct TextVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
	};

	struct LineVertex
	{
		//holds useless information, but that info will be used for the same shader as quads use
		//so it still needs it
		//TODO: Rework lines to use diff shader?
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	


	struct RendererData2D
	{
		static const uint32_t MaxTextureSlots = 32;
		Ref<Texture2D> WhiteTexture;


		//QUADS

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotsIndex = 1;
		Ref<Shader> TextureShader;


		static const uint32_t MaxQuads = 40000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
		glm::vec4 QuadVertexPositions[4] = {
			{-0.5f, -0.5f, 0, 1},//bl
			{ 0.5f, -0.5f, 0, 1},//br
			{ 0.5f,  0.5f, 0, 1},//tr
			{-0.5f,  0.5f, 0, 1} //tl
			
		};
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;

		//BITMAPS

		std::array<Ref<Texture2D>, MaxTextureSlots> BitMapTextureSlots;
		uint32_t BitMapSlotsIndex = 0;
		Ref<Shader> BitMapShader;

		static const uint32_t MaxBitmapQuads = 40000;
		static const uint32_t MaxBitmapQuadsVertices = MaxBitmapQuads * 4;
		static const uint32_t MaxBitmapIndices = MaxBitmapQuads * 6;

		Ref<VertexArray> BitMapVertexArray;
		Ref<VertexBuffer> BitMapVertexBuffer;
		Ref<IndexBuffer> BitMapIndexBuffer;
		glm::vec4 BitMapVertexPositions[4] = {
			{ 0, 0, 0, 1},//bl
			{ 1, 0, 0, 1},//br
			{ 1, 1, 0, 1},//tr
			{ 0, 1, 0, 1} //tl
		};
		glm::vec2 BitMapVertexTexCoords[4] = {
			{0,1},//bl
			{1,1},//br
			{1,0},//tr
			{0,0}//tl
		};

		uint32_t BitMapIndexCount = 0;
		TextVertex* BitMapVertexBufferBase = nullptr;
		TextVertex* BitMapVertexBufferPtr = nullptr;

		
		//LINES
		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		glm::vec4 LineVertexPositions[2] = {
			{ -0.5f, 0.0f, 0.0f, 1},
			{ 0.5f, 0.0f, 0.0f, 1 }
		};
		LineVertex LineVertexBufferData[2];

		Renderer2D::Statistics Stats;
	};

	

	static RendererData2D s_Data; // can make this a pointer

	void Renderer2D::Init()
	{
		//initialize the renderer2d primitive things
		//s_Data = new RendererData2D();
		
		////////////////////
		/// QUADS
		////////////////////

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
			{ShaderDataType::Uint, "a_NodeID"},
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

		////////////////////
		/// TEXT
		////////////////////

		
		s_Data.BitMapShader = Shader::Create("assets/shaders/Text.glsl");
		s_Data.BitMapShader->Bind();
		s_Data.BitMapShader->SetIntArray("u_BitMapTextures", samplers, s_Data.MaxTextureSlots);

		s_Data.BitMapVertexArray = VertexArray::Create();
		s_Data.BitMapVertexBuffer = VertexBuffer::Create(s_Data.MaxBitmapQuadsVertices * sizeof(TextVertex));

		BufferLayout TextBufferLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexID"},
		};
		s_Data.BitMapVertexBuffer->SetLayout(TextBufferLayout);
		s_Data.BitMapVertexArray->AddVertexBuffer(s_Data.BitMapVertexBuffer);
		s_Data.BitMapVertexBufferBase = new TextVertex[s_Data.MaxBitmapQuadsVertices];


		//set indices
		uint32_t* BitMapIndices = new uint32_t[s_Data.MaxBitmapIndices];
		uint32_t BitMapOffset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			BitMapIndices[i + 0] = BitMapOffset + 0;
			BitMapIndices[i + 1] = BitMapOffset + 1;
			BitMapIndices[i + 2] = BitMapOffset + 2;

			BitMapIndices[i + 3] = BitMapOffset + 2;
			BitMapIndices[i + 4] = BitMapOffset + 3;
			BitMapIndices[i + 5] = BitMapOffset + 0;

			BitMapOffset += 4;
		}

		s_Data.BitMapIndexBuffer = IndexBuffer::Create(BitMapIndices, s_Data.MaxBitmapIndices);
		s_Data.BitMapVertexArray->SetIndexBuffer(s_Data.BitMapIndexBuffer);
		delete[] BitMapIndices;


		////////////////////
		/// LINES
		////////////////////
		s_Data.LineVertexArray = VertexArray::Create();
		s_Data.LineVertexBuffer = VertexBuffer::Create(2 * sizeof(LineVertex));

		s_Data.LineVertexBuffer->SetLayout(layout);
		s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);

		s_Data.LineVertexBufferData[0].Position = s_Data.LineVertexPositions[0];
		s_Data.LineVertexBufferData[0].Color = {1,1,1,1};
		s_Data.LineVertexBufferData[0].TexCoord = {0,0};
		s_Data.LineVertexBufferData[0].TexIndex = 0.0f;
		s_Data.LineVertexBufferData[0].TilingFactor = 1;

		s_Data.LineVertexBufferData[1].Position = s_Data.LineVertexPositions[1];
		s_Data.LineVertexBufferData[1].Color = { 1,1,1,1 };
		s_Data.LineVertexBufferData[1].TexCoord = { 1,1 };
		s_Data.LineVertexBufferData[1].TexIndex = 0.0f;
		s_Data.LineVertexBufferData[1].TilingFactor = 1;


		//texture init
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

	void Renderer2D::BeginScene(const Ref<Pyxis::Camera> camera)
	{	
		s_Data.BitMapShader->Bind();
		s_Data.BitMapShader->SetMat4("u_ViewProjection", camera->GetViewProjectionMatrix());

		s_Data.BitMapVertexBufferPtr = s_Data.BitMapVertexBufferBase;
		s_Data.BitMapIndexCount = 0;

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera->GetViewProjectionMatrix());

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
		////////////////////////////
		/// Flush normal quads first
		////////////////////////////
		s_Data.TextureShader->Bind();
		//send data to buffer
		uint32_t size = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, size);

		for (uint32_t i = 0; i < s_Data.TextureSlotsIndex; i++)
		{
			s_Data.TextureSlots[i]->Bind(i);
		}
		//draw
		if (s_Data.QuadIndexCount > 0)
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotsIndex = 1;

		///////////////////////
		/// Flush BitMap Quads
		///////////////////////

		s_Data.BitMapShader->Bind();
		//send data to buffer
		uint32_t BitMapBufferSize = (uint8_t*)s_Data.BitMapVertexBufferPtr - (uint8_t*)s_Data.BitMapVertexBufferBase;
		s_Data.BitMapVertexBuffer->SetData(s_Data.BitMapVertexBufferBase, BitMapBufferSize);

		for (uint32_t i = 0; i < s_Data.BitMapSlotsIndex; i++)
		{
			s_Data.BitMapTextureSlots[i]->Bind(i);
		}
		//draw
		if (s_Data.BitMapIndexCount > 0)
			RenderCommand::DrawIndexed(s_Data.BitMapVertexArray, s_Data.BitMapIndexCount);
		s_Data.BitMapVertexBufferPtr = s_Data.BitMapVertexBufferBase;
		s_Data.BitMapIndexCount = 0;
		s_Data.BitMapSlotsIndex = 0;


#if STATISTICS
		s_Data.Stats.DrawCalls++;
#endif
	}

	void Renderer2D::DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color)
	{
		DrawLine({ start.x, start.y, 0 }, { end.x, end.y, 0 }, color);
	}

	void Renderer2D::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
	{
		s_Data.LineVertexBufferData[0].Position = start;
		s_Data.LineVertexBufferData[0].Color = color;
		s_Data.LineVertexBufferData[1].Position = end;
		s_Data.LineVertexBufferData[1].Color = color;
		s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferData, 2 * sizeof(LineVertex));
		s_Data.TextureSlots[0]->Bind();
		RenderCommand::DrawLines(s_Data.LineVertexArray, 2);
	}

	/// <summary>
	/// Submit a transform and color to be drawn
	/// </summary>
	void Renderer2D::DrawQuad(glm::mat4 transform, const glm::vec4& color)
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
			s_Data.QuadVertexBufferPtr->TilingFactor = 1;
			s_Data.QuadVertexBufferPtr->NodeID = 0;
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
			s_Data.QuadVertexBufferPtr->NodeID = 0;
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
			s_Data.QuadVertexBufferPtr->NodeID = 0;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}


	void Renderer2D::DrawQuad(const glm::vec2 position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0 }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3 position, const glm::vec2& size, const glm::vec4& color)
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
			s_Data.QuadVertexBufferPtr->TilingFactor = 1;
			s_Data.QuadVertexBufferPtr->NodeID = 0;
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
			s_Data.QuadVertexBufferPtr->NodeID = 0;
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
			s_Data.QuadVertexBufferPtr->NodeID = 0;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2 position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0 }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3 position, const glm::vec2& size, float rotation, const glm::vec4& color)
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
			s_Data.QuadVertexBufferPtr->TilingFactor = 1;
			s_Data.QuadVertexBufferPtr->NodeID = 0;
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
			s_Data.QuadVertexBufferPtr->NodeID = 0;
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
			s_Data.QuadVertexBufferPtr->NodeID = 0;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawQuadEntity(const glm::vec3 position, const glm::vec2& size, const glm::vec4& color, uint32_t nodeID)
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
			s_Data.QuadVertexBufferPtr->TilingFactor = 1;
			s_Data.QuadVertexBufferPtr->NodeID = nodeID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawQuadEntity(const glm::vec3 position, const glm::vec2& size, const Ref<Texture2D>& texture, uint32_t nodeID, float tilingFactor, const glm::vec4& tintColor)
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
			s_Data.QuadVertexBufferPtr->NodeID = nodeID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawQuadEntity(glm::mat4 transform, const glm::vec4& color, uint32_t nodeID)
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
			s_Data.QuadVertexBufferPtr->TilingFactor = 1;
			s_Data.QuadVertexBufferPtr->NodeID = nodeID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawQuadEntity(glm::mat4 transform, const Ref<Texture2D>& texture, uint32_t nodeID, float tilingFactor, const glm::vec4& tintColor)
	{
		//check if we need to flush
		if (s_Data.QuadIndexCount >= RendererData2D::MaxIndices)
		{
			Flush();
		}

		//texture coords
		glm::vec2 textureCoords[4] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		
		for (int i = 0; i < 4; i++)
		{
			glm::vec2 TiledCoord = (transform * s_Data.QuadVertexPositions[i]) - (transform * s_Data.QuadVertexPositions[0]);
			if (texture->GetTextureSpecification().m_TextureModeS == Texture::TextureMode::Tile) textureCoords[i].x = TiledCoord.x;
			if (texture->GetTextureSpecification().m_TextureModeT == Texture::TextureMode::Tile) textureCoords[i].y = TiledCoord.y;
		}

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
			s_Data.QuadVertexBufferPtr->NodeID = nodeID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	void Renderer2D::DrawQuadEntity(glm::mat4 transform, const Ref<SubTexture2D>& subTexture, uint32_t nodeID, float tilingFactor, const glm::vec4& tintColor)
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
			s_Data.QuadVertexBufferPtr->NodeID = nodeID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
#if STATISTICS
		s_Data.Stats.QuadCount++;
#endif
	}

	/// <summary>
	/// Draws just like a quad, but uses a bitmap (only red channel).
	/// Has a separate storage buffer and attempts to render after for transparency
	/// </summary>
	void Renderer2D::DrawBitMap(glm::mat4 transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor)
	{
		//check if we hit text limit
		if (s_Data.BitMapIndexCount >= RendererData2D::MaxBitmapIndices)
		{
			Flush();
		}

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 0; i < s_Data.BitMapSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.BitMapTextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
		}
		if (TexIndex == 0.0f)
		{
			if (s_Data.BitMapSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.BitMapSlotsIndex;
			s_Data.BitMapTextureSlots[s_Data.BitMapSlotsIndex] = texture;
			s_Data.BitMapSlotsIndex = s_Data.BitMapSlotsIndex + 1;
		}

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.BitMapVertexBufferPtr->Position = transform * s_Data.BitMapVertexPositions[i];
			s_Data.BitMapVertexBufferPtr->Color = tintColor;
			s_Data.BitMapVertexBufferPtr->TexCoord = s_Data.BitMapVertexTexCoords[i];
			s_Data.BitMapVertexBufferPtr->TexIndex = TexIndex;
			s_Data.BitMapVertexBufferPtr++;
		}
		s_Data.BitMapIndexCount += 6;

	}

	void Renderer2D::DrawBitMap(glm::mat4 transform, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor)
	{
		//check if we hit text limit
		if (s_Data.BitMapIndexCount >= RendererData2D::MaxBitmapIndices)
		{
			Flush();
		}

		//texture coords
		const glm::vec2* textureCoords = subTexture->GetTexCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		//check if the texture can fit into GPU, or already is there
		float TexIndex = 0.0f;
		for (uint32_t i = 0; i < s_Data.BitMapSlotsIndex; i++)
		{
			//check if we are already in there
			if (*s_Data.BitMapTextureSlots[i].get() == *texture.get())
			{
				TexIndex = (float)i;
				break;
			}
		}
		if (TexIndex == 0.0f)
		{
			if (s_Data.BitMapSlotsIndex == s_Data.MaxTextureSlots)
			{
				Flush();
			}
			TexIndex = (float)s_Data.BitMapSlotsIndex;
			s_Data.BitMapTextureSlots[s_Data.BitMapSlotsIndex] = texture;
			s_Data.BitMapSlotsIndex = s_Data.BitMapSlotsIndex + 1;
		}

		constexpr size_t quadVertexCount = 4;
		for (int i = 0; i < quadVertexCount; i++)
		{
			s_Data.BitMapVertexBufferPtr->Position = transform * s_Data.BitMapVertexPositions[i];
			s_Data.BitMapVertexBufferPtr->Color = tintColor;
			s_Data.BitMapVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.BitMapVertexBufferPtr->TexIndex = TexIndex;
			s_Data.BitMapVertexBufferPtr++;
		}
		s_Data.BitMapIndexCount += 6;

	}


	void Renderer2D::DrawText(const std::string& text, glm::mat4 transform, Ref<Font> font, float fontSize, float lineHeight, float maxWidth, const glm::vec4& color)
	{
		float size = fontSize / 1000.0f;
		float newLineShift = lineHeight * size * font->m_CharacterHeight;
		// iterate through all characters
		glm::vec2 pos = {0,0};
		std::string::const_iterator c;

		std::vector<std::string> words;
		size_t index = text.find(' ');
		size_t initialPos = 0;
		while (index != std::string::npos) {
			//create the word substring
			words.push_back(text.substr(initialPos, index - initialPos));
			initialPos = index + 1;
			index = text.find(' ', initialPos);
		}
		// Add the last one
		int len = std::min(index, text.size()) - initialPos + 1;
		if (len > 0)
			words.push_back(text.substr(initialPos, len));
		
		for (std::string& word : words)
		{

			//PX_TRACE("Word: {0}", word);

			if (word == "")
			{
				pos.x += (font->m_Characters[' '].Advance >> 6) * size * 2;
				continue;
			}

			float wordLength = 0;
			for (auto c : word)
			{
				wordLength += (font->m_Characters[c].Advance >> 6) * size;
			}
			if ((wordLength + pos.x) > maxWidth * size * (font->m_Characters[' '].Advance >> 6))
			{
				//this word makes it go past the max length. we need to do a new line before this word
				//(new line)
				pos.x = 0;
				pos.y -= newLineShift;
			}
			for (c = word.begin(); c != word.end(); c++)
			{
				if (*c == '\n')
				{
					//(new line)
					pos.x = 0;
					pos.y -= newLineShift;
					continue;
				}

				Font::Character ch = font->m_Characters[*c];
				//create a transform for the character
				float xpos = pos.x + ch.Bearing.x * size;
				float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * size;

				glm::mat4 charTransform = glm::translate(glm::mat4(1), { xpos, ypos, 0 });
				charTransform = glm::scale(charTransform, { ch.Size.x * size, ch.Size.y * size, 1 });
				charTransform = transform * charTransform;

				DrawBitMap(charTransform, ch.Texture, color);

				// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				pos.x += (ch.Advance >> 6) * size; // bitshift by 6 to get value in pixels (2^6 = 64)
			}
			//put the space for a space
			pos.x += (font->m_Characters[' '].Advance >> 6) * size * 2;
		}
		
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