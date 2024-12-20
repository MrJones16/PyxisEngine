#pragma once

#include "Camera.h"
#include "Texture.h"
#include "SubTexture2D.h"

namespace Pyxis
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Ref<Pyxis::OrthographicCamera> camera);
		static void EndScene();
		static void Flush();

		//Primitives

		static void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color = { 1,1,1,1 });
		static void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = { 1,1,1,1 });
		
		static void DrawQuad(glm::mat4 transform, const glm::vec4& color, float tilingFactor = 1);
		static void DrawQuad(glm::mat4 transform, const Ref<Texture2D>& texture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });
		static void DrawQuad(glm::mat4 transform, const Ref<SubTexture2D>& subTexture, float tilingFactor = 1, const glm::vec4& tintColor = {1,1,1,1});


		static void DrawQuad(const glm::vec2 position, const glm::vec2& size, const glm::vec4& color, float tilingFactor = 1);
		static void DrawQuad(const glm::vec3 position, const glm::vec2& size, const glm::vec4& color, float tilingFactor = 1);
		static void DrawQuad(const glm::vec2 position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });
		static void DrawQuad(const glm::vec3 position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });
		static void DrawQuad(const glm::vec2 position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });
		static void DrawQuad(const glm::vec3 position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });

		static void DrawRotatedQuad(const glm::vec2 position, const glm::vec2& size, float rotation, const glm::vec4& color, float tilingFactor = 1);
		static void DrawRotatedQuad(const glm::vec3 position, const glm::vec2& size, float rotation, const glm::vec4& color, float tilingFactor = 1);
		static void DrawRotatedQuad(const glm::vec2 position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });
		static void DrawRotatedQuad(const glm::vec3 position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });
		static void DrawRotatedQuad(const glm::vec2 position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });
		static void DrawRotatedQuad(const glm::vec3 position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor = 1, const glm::vec4& tintColor = { 1,1,1,1 });

		//Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static void ResetStats();
		static Statistics GetStats();
	};
}