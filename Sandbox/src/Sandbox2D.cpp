#include "Sandbox2D.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>

template<typename Fn>
class Timer
{
public:
	Timer(const char* name, Fn&& func)
		: m_Name(name), m_Stopped(false), m_Func(func)
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		if (!m_Stopped)
			Stop();
	}

	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();
		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		m_Stopped = true;
		float duration = (end - start) * 0.001f;

		m_Func({ m_Name, duration });
		//std::cout << m_Name << ": Duration: " << duration << "milliseconds" << std::endl;
	}
private:
	const char* m_Name;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	bool m_Stopped;
	Fn m_Func;
};

#if PROFILING
	#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfileResults.push_back(profileResult);})
#else
	#define PROFILE_SCOPE(name)
#endif

Sandbox2D::Sandbox2D()
	: Layer("Sandbox"), 
	m_OrthographicCameraController(5, 9.0f / 16.0f, -100, 100)
{

}

void Sandbox2D::OnAttach()
{
	//m_ProfileResults = std::vector<ProfileResult>();
	Pyxis::Renderer2D::Init();
	m_TestTexture = Pyxis::Texture2D::Create("assets/textures/bluemush.png");
	m_SpritesheetTexture = Pyxis::Texture2D::Create("assets/textures/ForestTileSet.png");
	m_SubTextureTest = Pyxis::SubTexture2D::CreateFromCoords(m_SpritesheetTexture, { 0,0 }, { 16,16 });
}

void Sandbox2D::OnDetatch()
{
}

void Sandbox2D::OnUpdate(Pyxis::Timestep ts)
{
	PROFILE_SCOPE("Sandbox2D::OnUpdate");
	//update
	m_OrthographicCameraController.OnUpdate(ts);

	//rendering
#if STATISTICS
	Pyxis::Renderer2D::ResetStats();
#endif
	
	{
		PROFILE_SCOPE("Renderer Prep");
		Pyxis::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Pyxis::RenderCommand::Clear();
		Pyxis::Renderer2D::BeginScene(m_OrthographicCameraController.GetCamera());
	}

	{
		PROFILE_SCOPE("Renderer Draw");
		Pyxis::Renderer2D::DrawRotatedQuad(m_TestPosition, m_TestSize, glm::radians(m_TestRotation), m_TestColor);

		for (float x = -5.0f; x < 5.0f; x += 0.5f)
		{
			for (float y = -5.0f; y < 5.0f; y += 0.5f)
			{
				Pyxis::Renderer2D::DrawQuad({ x, y , -1}, { 0.08f, 0.08f }, { (x + 5) / 10, (y + 5) / 10 , 1, 1 });
			}
		}
		Pyxis::Renderer2D::DrawQuad({ 2.0f, 0.0f, 0.0f }, { 1.0f ,1.0f }, m_TestTexture);

		Pyxis::Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, m_SubTextureTest);
	}

	//m_FrameBuffer->Unbind()
	Pyxis::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Testing");

	ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_TestColor), 0.1f);
	ImGui::DragFloat3("SquarePosition", glm::value_ptr(m_TestPosition), 0.1f);
	ImGui::DragFloat2("SquareSize", glm::value_ptr(m_TestSize), 0.1f);
	ImGui::DragFloat("SquareRotation", &m_TestRotation, 0.1f);
	ImGui::End();

	//profiling
	const int AverageAmount = 100;
	ImGui::Begin("Profiler");

	if (m_ProfileAverageValue.empty())
	{
		for (auto& result : m_ProfileResults)
		{
			m_ProfileAverageValue.push_back(result.Time);
		}
	}
	auto avgVal = m_ProfileAverageValue.begin();
	//update the average with current time
	for (auto& result : m_ProfileResults)
	{
		*avgVal = *avgVal + result.Time;
		avgVal++;
	}
	//check for every 100th update, and update storage
	if (m_ProfileAverageCount >= AverageAmount)
	{
		m_ProfileAverageValueStorage.clear();
		avgVal = m_ProfileAverageValue.begin();
		for (auto& result : m_ProfileResults)
		{
			//set the storage
			m_ProfileAverageValueStorage.push_back(*avgVal / AverageAmount);
			avgVal++;
		}
		m_ProfileAverageValue.clear();
		m_ProfileAverageCount = 0;
	}
	//loop back through profiles and print times
	auto avgValFromStorage = m_ProfileAverageValueStorage.begin();
	for (auto& result : m_ProfileResults)
	{
		//write the average
		char label[50];
		strcpy(label, "%.3fms ");
		strcat(label, result.Name);
		ImGui::Text(label, *avgValFromStorage);
		avgValFromStorage++;
	}
	m_ProfileAverageCount++;
	m_ProfileResults.clear();
	ImGui::End();

#if STATISTICS
	Pyxis::Renderer2D::Statistics stats = Pyxis::Renderer2D::GetStats();
	ImGui::Begin("Rendering Statistics");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::End();
#endif

	//ImGui::Begin("Scene");
	//{
	//	//ImGui::GetForegroundDrawList()->AddRect(ImVec2(0, 0), windowSize, ImU32(0xFFFFFFFF));
	//	ImGui::BeginChild("GameRender");
	//	ImVec2 windowSize = ImGui::GetContentRegionMax();

	//	m_OrthographicCameraController.SetAspect(windowSize.y / windowSize.x);
	//	//PX_CORE_INFO("{0}, {1}", windowSize.x, windowSize.y);
	//	Pyxis::Renderer::OnWindowResize(windowSize.x, windowSize.y);
	//	ImGui::Image(
	//		(ImTextureID)m_FrameBuffer->GetFrameBufferTexture()->GetID(),
	//		ImGui::GetContentRegionAvail(),
	//		ImVec2(0, 1),
	//		ImVec2(1, 0),
	//		ImVec4(1, 1, 1, 1)
	//		//ImVec4(1, 1, 1, 1) border color
	//	);
	//}
	//ImGui::EndChild();
	//ImGui::End();
}

void Sandbox2D::OnEvent(Pyxis::Event& e)
{
	m_OrthographicCameraController.OnEvent(e);
	Pyxis::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Pyxis::WindowResizeEvent>(PX_BIND_EVENT_FN(Sandbox2D::OnWindowResizeEvent));
}

bool Sandbox2D::OnWindowResizeEvent(Pyxis::WindowResizeEvent& event) {
	m_OrthographicCameraController.SetAspect((float)event.GetHeight() / (float)event.GetWidth());
	Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	//Pyxis::Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	return false;
}