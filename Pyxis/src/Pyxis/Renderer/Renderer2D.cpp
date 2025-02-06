#include "pxpch.h"
#include "Renderer2D.h"
#include "RenderCommand.h"

#include "VertexArray.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>

namespace Pyxis
{

	namespace Utils
	{
		//possible Optimization: instead of using find(), iterate manually and get the word and word length in one go.

		//Gets a vector of words (a word being an std::pair with the word, and the physical length of the word)
		std::vector<Word> TextToWords(const std::string& text, Ref<Font> font, float fontSize)
		{
			std::vector<Word> result;
			size_t index = std::min(text.find(' '), text.find('\n'));
			size_t initialPos = 0;
			while (index != std::string::npos) {
				//create the word substring
				bool hasNewLine = text[index] == '\n';
				result.push_back({ text.substr(initialPos, index - initialPos), 0 , hasNewLine });
				initialPos = index + 1;
				index = std::min(text.find(' ', initialPos), text.find('\n', initialPos));
			}
			// Add the last one
			int len = std::min(index, text.size()) - initialPos + 1;
			if (len > 0)
				result.push_back({ text.substr(initialPos, len), 0 , false });

			for (auto& word : result)
			{
				if (word.string == "")
				{
					word.physicalLength = (font->m_Characters[' '].Advance >> 6) * fontSize * 2;
					continue;
				}
				//get the length of the word
				for (auto c : word.string)
				{
					if (c == '\n')
					{
						word.ContainsNewLine = true;
					}
					word.physicalLength += (font->m_Characters[c].Advance >> 6) * fontSize;
				}
			}
			return result;
		}

	}

	struct ScreenQuadVertex
	{
		glm::vec2 Position;
		glm::vec2 TexCoord;
	};

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
		uint32_t NodeID;
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

		Ref<Shader> ScreenQuadShader;
		Ref<VertexArray> ScreenQuadVertexArray;
		Ref<VertexBuffer> ScreenQuadVertexBuffer;
		Ref<IndexBuffer> ScreenQuadIndexBuffer;


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

		//texture init
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t WhiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&WhiteTextureData, sizeof(WhiteTextureData));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		//screen quad
		s_Data.ScreenQuadShader = Shader::Create("assets/shaders/ScreenQuad.glsl");
		s_Data.ScreenQuadShader->Bind();
		//s_Data.ScreenQuadShader->SetInt("u_Texture", s_Data.WhiteTexture->GetID());

		s_Data.ScreenQuadVertexArray = VertexArray::Create();
		s_Data.ScreenQuadVertexBuffer = VertexBuffer::Create(16 * sizeof(float));
		BufferLayout ScreenQuadLayout = {
			{ShaderDataType::Float2, "a_Position"},
			{ShaderDataType::Float2, "a_TexCoord"},
		};
		s_Data.ScreenQuadVertexBuffer->SetLayout(ScreenQuadLayout);
		ScreenQuadVertex ScreenQuadData[4] =
		{
			{{-1,-1}, {0,0}},
			{{1,-1}, {1,0}},
			{{1,1}, {1,1}},
			{{-1,1}, {0,1}}

		};


		uint32_t SingleQuadIndices[6] =
		{
			0,1,2,2,3,0
		};
		s_Data.ScreenQuadIndexBuffer = IndexBuffer::Create(SingleQuadIndices, 6);

		s_Data.ScreenQuadVertexBuffer->SetData(ScreenQuadData, 4 * sizeof(ScreenQuadVertex));
		s_Data.ScreenQuadVertexArray->AddVertexBuffer(s_Data.ScreenQuadVertexBuffer);
		s_Data.ScreenQuadVertexArray->SetIndexBuffer(s_Data.ScreenQuadIndexBuffer);
		
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
			{ShaderDataType::Uint, "a_NodeID"},
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

	}

	void Renderer2D::Shutdown()
	{
		//delete s_Data;
	}

	void Renderer2D::BeginScene(Pyxis::Camera* camera)
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
		RenderCommand::DisableDepthTesting();

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
		RenderCommand::EnableDepthTesting();


#if STATISTICS
		s_Data.Stats.DrawCalls++;
#endif
	}

	void Renderer2D::DrawScreenQuad(uint32_t TextureID)
	{
		RenderCommand::Clear();
		s_Data.ScreenQuadShader->Bind();
		s_Data.ScreenQuadVertexArray->Bind();
		RenderCommand::BindTexture2D(TextureID);
		//s_Data.ScreenQuadShader->SetInt("u_Texture", TextureID);
		RenderCommand::DrawIndexed(s_Data.ScreenQuadVertexArray, 6);
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
		glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
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
		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, { size.x, size.y, 0 });

		//texture coords
		glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
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

		//position
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, -rotation, { 0,0,1 });
		transform = glm::scale(transform, { size.x, size.y, 0 });

		//texture coords
		glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
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
		glm::vec2 textureCoords[] = { { 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		
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
	void Renderer2D::DrawBitMap(glm::mat4 transform, const Ref<Texture2D>& texture, uint32_t nodeID, const glm::vec4& tintColor)
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
			s_Data.BitMapVertexBufferPtr->NodeID = nodeID;
			s_Data.BitMapVertexBufferPtr++;
		}
		s_Data.BitMapIndexCount += 6;

	}

	void Renderer2D::DrawBitMap(glm::mat4 transform, const Ref<SubTexture2D>& subTexture, uint32_t nodeID, const glm::vec4& tintColor)
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
			s_Data.BitMapVertexBufferPtr->NodeID = nodeID;
			s_Data.BitMapVertexBufferPtr++;
		}
		s_Data.BitMapIndexCount += 6;

	}

	void Renderer2D::DrawText(const std::string& text, glm::mat4 transform, Ref<Font> font, float fontSize, float lineHeight, float maxWidth, UI::Direction alignment, const glm::vec4& color, uint32_t nodeID)
	{

		//method:
		// gets all the words in text seperated into list
		// sees how many words will fit into a single line or until new line
		// writes those words until finished
		// gets next set of words (repeat)
		//

		float size = fontSize;
		float newLineShift = lineHeight * size * font->m_CharacterHeight;
		float spaceWidth = (font->m_Characters[' '].Advance >> 6) * size * 2;

		// Main Writing Position
		glm::vec2 pos = { 0,0 };

		//gather words, their lengths, and if they have a new line
		std::vector<Word> words = Utils::TextToWords(text, font, fontSize);

		//the word we are at in the iteration 
		int wordIndex = 0;

		//loop until we write all words
		while (wordIndex < words.size())
		{
			//vars to track how many we can write in one go
			float lengthSum = 0;
			int wordsToWrite = 0;
			//count how many words we can write based on length
			for (int i = wordIndex; i < words.size(); i++)
			{
				if ((lengthSum + words[i].physicalLength) > maxWidth && wordsToWrite > 0) // > 0 keeps track if this is the first word
				{
					//this word won't fit with how many we have
					break;
				}
				else
				{
					wordsToWrite++;
					lengthSum += words[i].physicalLength + spaceWidth;
					if (words[i].ContainsNewLine)
						break;
					
				}
			}

			//we now have how many words we should write, so lets iterate over them and write!
			int maxIndex = wordsToWrite + wordIndex;
			for (int i = wordIndex; (i < maxIndex) && (i < words.size()); i++)
			{
				for (std::string::const_iterator c = words[i].string.begin(); c != words[i].string.end(); c++)
				{
					Font::Character ch = font->m_Characters[*c];
					//create a transform for the character
					float xpos = pos.x + ch.Bearing.x * size;
					float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * size;

					glm::mat4 charTransform = glm::translate(glm::mat4(1), { xpos, ypos, 0 });
					charTransform = glm::scale(charTransform, { ch.Size.x * size, ch.Size.y * size, 1 });
					charTransform = transform * charTransform;

					DrawBitMap(charTransform, ch.Texture, nodeID, color);

					// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
					pos.x += (ch.Advance >> 6) * size; // bitshift by 6 to get value in pixels (2^6 = 64)
				}
				//put the space for a space
				pos.x += spaceWidth;
				wordIndex++;
			}
			//(new line)
			pos.x = 0;
			pos.y -= newLineShift;

		}
	}

	void Renderer2D::DrawTextLine(const std::string& text, glm::mat4 transform, Ref<Font> font, const glm::vec2& maxSize, float fontSize, UI::Direction alignment, bool scaleToFit, const glm::vec4& color, uint32_t nodeID)
	{
		float spaceWidth = (font->m_Characters[' '].Advance >> 6) * fontSize * 2;

		//main writing position
		glm::vec2 pos = { 0,0 };

		std::vector<Word> words = Utils::TextToWords(text, font, fontSize);
		float totalLength = -spaceWidth; // neg space width for first word not having a space
		for (auto& word : words)
		{
			totalLength += spaceWidth + word.physicalLength;
		}
		

		float size = fontSize;
		if (scaleToFit)
		{
			//float lengthWithSpaces = totalLength + (words.size() - 1) * spaceWidth;
			
			float scaleFactor = std::min(maxSize.x / totalLength, maxSize.y / (font->m_CharacterHeight * fontSize));
			size *= scaleFactor;
			spaceWidth = (font->m_Characters[' '].Advance >> 6) * size * 2;
			totalLength *= scaleFactor;
		}


		if (alignment == UI::Direction::Left)
		{
			pos.x -= (maxSize.x / 2.0f);
		}
		else if (alignment == UI::Direction::Center)
		{
			pos.x -= totalLength / 2.0f;
		}

		for (int i = 0; i < words.size(); i++)
		{
			for (std::string::const_iterator c = words[i].string.begin(); c != words[i].string.end(); c++)
			{
				Font::Character ch = font->m_Characters[*c];
				//skip if this character is missing from the font
				if (ch.Texture == nullptr) continue;


				//create a transform for the character
				float xpos = pos.x + ch.Bearing.x * size;
				float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * size;
				ypos -= font->m_CharacterHeight * size * 0.77f;
				

				glm::mat4 charTransform = glm::translate(glm::mat4(1), { xpos, ypos, 0 });
				charTransform = glm::scale(charTransform, { ch.Size.x * size, ch.Size.y * size, 1 });
				charTransform = transform * charTransform;


				DrawBitMap(charTransform, ch.Texture, nodeID, color);

				// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				pos.x += (ch.Advance >> 6) * size; // bitshift by 6 to get value in pixels (2^6 = 64)
			}
			//put the space for a space
			pos.x += spaceWidth;
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