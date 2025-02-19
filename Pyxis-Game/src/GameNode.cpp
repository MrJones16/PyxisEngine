#include "GameNode.h"

#include <glm/gtc/type_ptr.hpp>

#include <Platform/OpenGL/OpenGLShader.h>
#include <chrono>
#include <variant>
#include "MenuNode.h"
#include "PixelBody2D.h"
#include <Pyxis/Game/Physics2D.h>
#include <snappy.h>

namespace Pyxis
{

	GameNode::GameNode(std::string debugName)
		: Node(debugName),
		m_KeyPressedReciever(this, &GameNode::OnKeyPressedEvent),
		m_MouseButtonPressedReciever(this, &GameNode::OnMouseButtonPressedEvent),
		m_MouseScrolledReciever(this, &GameNode::OnMouseScrolledEvent)
	{
		EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);
		EventSignal::s_MouseButtonPressedEventSignal.AddReciever(m_MouseButtonPressedReciever);
		EventSignal::s_MouseScrolledEventSignal.AddReciever(m_MouseScrolledReciever);

		//Set up the UI Heirarchy since we have no scenes

		//I'm going to aim for a bottom hot bar for now.
		m_CameraController = Instantiate<OrthographicCameraControllerNode>();
		m_CameraController->SetMainCamera();
		m_CameraController->SetWidth(2);
		m_CameraController->Translate({ 0,1,0 });
		AddChild(m_CameraController);

		auto screenSpace = Instantiate<UI::ScreenSpace>();
		AddChild(screenSpace);

		m_Hotbar = Instantiate<UI::Canvas>();
		m_Hotbar->m_AutomaticSizing = true;
		m_Hotbar->m_AutomaticSizingPercent = { 1, 1 };//10 % height
		m_Hotbar->m_FixedSize.y = 112;
		//full x, 1/10th up the screen
		m_Hotbar->m_AutomaticPositioning = true;
		m_Hotbar->m_VerticalAlignment = UI::Down;
		m_Hotbar->CreateTextures("assets/textures/UI/GreenCanvas/", "GreenCanvasTile_", ".png");
		m_Hotbar->m_PPU = 32;
		screenSpace->AddChild(m_Hotbar);


		auto container = Instantiate<UI::Container>();
		container->m_AutomaticSizing = true;
		container->m_AutomaticSizingPercent = { 1,1 };
		container->m_AutomaticSizingOffset = { -32, -32 };
		container->m_Gap = 32;
		container->m_VerticalAlignment = UI::Center;
		container->Translate({ 0,0,-0.05f });
		m_Hotbar->AddChild(container);

		///Pause & Play Buttons
		auto ButtonHolder = Instantiate<UI::UIRect>("Button Holder");
		ButtonHolder->m_Enabled = false;
		ButtonHolder->m_Size = { 32, 32 };
		m_PlayButton = Instantiate<UI::Button>("Play Button", ResourceManager::Load<Texture2DResource>("assets/Textures/UI/PlayButton.png"));
		m_PlayButton->SetFunction(std::bind(&GameNode::PlayButtonFunc, this));
		m_PlayButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/Textures/UI/PlayButtonPressed.png");
		m_PlayButton->m_Size = { 32,32 };
		m_PlayButton->m_Enabled = false;
		ButtonHolder->AddChild(m_PlayButton);
		m_PauseButton = Instantiate<UI::Button>("Pause Button", ResourceManager::Load<Texture2DResource>("assets/Textures/UI/PauseButton.png"));
		m_PauseButton->SetFunction(std::bind(&GameNode::PauseButtonFunc, this));
		m_PauseButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/Textures/UI/PauseButtonPressed.png");
		m_PauseButton->m_Size = { 32,32 };
		ButtonHolder->AddChild(m_PauseButton);
		container->AddChild(ButtonHolder);

		//Brush Buttons
		auto BrushOptions = Instantiate<UI::UIRect>("Button Holder");
		BrushOptions->m_Enabled = false;
		BrushOptions->m_Size = { 72, 32 };
		auto brushCircle = Instantiate<UI::Button>("BrushCircle", ResourceManager::Load<Texture2DResource>("assets/Textures/UI/BrushCircleButton.png"));
		brushCircle->SetFunction(std::bind(&GameNode::SetBrushType, this, BrushType::circle));
		brushCircle->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/Textures/UI/BrushCircleButtonPressed.png");
		brushCircle->m_Size = { 32,32 };
		brushCircle->Translate({ -20, 0, 0 });
		BrushOptions->AddChild(brushCircle);

		auto brushSquare = Instantiate<UI::Button>("BrushSquare", ResourceManager::Load<Texture2DResource>("assets/Textures/UI/BrushSquareButton.png"));
		brushSquare->SetFunction(std::bind(&GameNode::SetBrushType, this, BrushType::square));
		brushSquare->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/Textures/UI/BrushSquareButtonPressed.png");
		brushSquare->m_Size = { 32,32 };
		brushSquare->Translate({ 20, 0, 0 });
		BrushOptions->AddChild(brushSquare);
		container->AddChild(BrushOptions);		

		auto ElementButtonContainer = Instantiate<UI::Container>("Element Buttons Container");
		//ElementButtonContainer->m_Size = {900, 32};
		ElementButtonContainer->m_AutomaticSizing = true;
		ElementButtonContainer->m_AutomaticSizingOffset = { -350, 0 };
		ElementButtonContainer->m_Gap = 0;
		for (int i = 0; i < m_World.m_ElementData.size(); i++)
		{
			ElementData& ed = m_World.m_ElementData[i];

			//abgr
			int r = (ed.color & 0x000000FF) >> 0;
			int g = (ed.color & 0x0000FF00) >> 8;
			int b = (ed.color & 0x00FF0000) >> 16;
			int a = (ed.color & 0xFF000000) >> 24;

			auto ElementTextButton = Instantiate<UI::TextButton>("ElementTextButton", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&GameNode::SetBrushElement, this, i));
			ElementTextButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateWhite.png");
			ElementTextButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateWhitePressed.png");
			ElementTextButton->m_TextBorderSize = glm::vec2(2);
			ElementTextButton->m_Color = glm::vec4(r, g, b, std::max(128, a)) / 255.0f;
			ElementTextButton->m_TextColor = glm::vec4(ElementTextButton->m_Color.r, ElementTextButton->m_Color.g, ElementTextButton->m_Color.b, 1);

			if (ElementTextButton->m_Color.r + ElementTextButton->m_Color.g + ElementTextButton->m_Color.b / 3.0f > 0.8f)
			{
				//element color is bright so make it darker
				ElementTextButton->m_TextColor = glm::vec4(ElementTextButton->m_Color.r * 0.1f, ElementTextButton->m_Color.g * 0.1f, ElementTextButton->m_Color.b * 0.1f, 1);
			}
			else
			{
				//its dark so make it brighter
				//ElementTextButton->m_TextColor = ElementTextButton->m_Color;
				for (int i = 0; i < 3 ;i++)
					ElementTextButton->m_TextColor[i] = std::min(ElementTextButton->m_TextColor[i] + 0.50f, 1.0f);
			}
			//ElementTextButton->m_TextColor.a = 255;
			ElementTextButton->m_PPU = 0.5f;
			ElementTextButton->UpdateSizeFromTexture();

			ElementTextButton->m_Text = ed.name;
			ElementTextButton->m_FontSize = 0.5f;
			ElementTextButton->Translate({ 0,0,-0.01f });			
			ElementButtonContainer->AddChild(ElementTextButton);
		}
		container->AddChild(ElementButtonContainer);

		//Quit Button		
		auto quitButton = Instantiate<UI::TextButton>("Quit Game Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&GameNode::ReturnToMenu, this));
		quitButton->m_PPU = 0.5;
		quitButton->m_Text = "Quit Game";
		quitButton->m_TextColor = glm::vec4(255.0f / 255.0f, 221.0f / 255.0f, 159.0f / 255.0f, 1);
		quitButton->Translate({ 0,0,1 });
		quitButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
		quitButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
		quitButton->UpdateSizeFromTexture();
		quitButton->m_TextBorderSize = glm::vec2(5,5);
		quitButton->m_TextOffset = { 0, 3, -0.0001f };
		quitButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
		container->AddChild(quitButton);


		screenSpace->PropagateUpdate(); // simple way to tell the hotbar to fix itself
	}

	GameNode::~GameNode()
	{
		
	}


	/// <summary>
	/// This is the main update loop involving actually playing the game, input, controls, ect.
	/// Also includes rendering loop
	/// 
	/// Only run this if m_World is valid and ready to be played!
	/// The output of inputs is in m_CurrentTickClosure
	/// </summary>
	/// <param name="ts"></param>
	void GameNode::GameUpdate(Timestep ts)
	{

		m_Hovering = (Node::s_HoveredNodeID == GetUUID()) || (Node::s_HoveredNodeID == 0);
		//PX_TRACE("Hovered Node: {0}", Node::s_HoveredNodeID);

		{
			PROFILE_SCOPE("Game Update");
			glm::ivec2 mousePixelPos = m_World.WorldToPixel(GetMousePosWorld());

			auto chunkPos = m_World.PixelToChunk(mousePixelPos);
			auto it = m_World.m_Chunks.find(chunkPos);
			if (it != m_World.m_Chunks.end())
			{
				auto index = m_World.PixelToIndex(mousePixelPos);
				m_HoveredElement = it->second->m_Elements[index.x + index.y * CHUNKSIZE];
				if (m_HoveredElement.m_ID >= m_World.m_TotalElements)
				{
					//something went wrong? how?
					m_HoveredElement = Element();
				}
			}
			else
			{
				m_HoveredElement = Element();
			}


			if (Input::IsMouseButtonPressed(0) && m_Hovering)
			{
				glm::vec2 mousePos = GetMousePosWorld();
				glm::ivec2 pixelPos = m_World.WorldToPixel(mousePos);
				if (m_BuildingRigidBody)
				{
					if (pixelPos.x < m_RigidMin.x) m_RigidMin.x = pixelPos.x;
					if (pixelPos.x > m_RigidMax.x) m_RigidMax.x = pixelPos.x;
					if (pixelPos.y < m_RigidMin.y) m_RigidMin.y = pixelPos.y;
					if (pixelPos.y > m_RigidMax.y) m_RigidMax.y = pixelPos.y;
				}
				else
				{
					m_CurrentTickClosure.AddInputAction(
						InputAction::Input_Place,
						(uint8_t)m_BrushSize,
						(uint16_t)m_BrushType,
						(uint32_t)m_SelectedElementIndex,
						pixelPos,
						false);
				}
			}
		}

		{
			PROFILE_SCOPE("Renderer Draw");
			m_World.RenderWorld();
			PaintBrushHologram();
			//draw rigid body outline
			//horizontals
			glm::vec2 worldMin = glm::vec2((float)m_RigidMin.x / (float)CHUNKSIZE, (float)m_RigidMin.y / (float)CHUNKSIZE);
			glm::vec2 worldMax = glm::vec2((float)m_RigidMax.x / (float)CHUNKSIZE, (float)m_RigidMax.y / (float)CHUNKSIZE);
			Renderer2D::DrawLine({ worldMin.x, worldMin.y }, { worldMax.x, worldMin.y});
			Renderer2D::DrawLine({ worldMin.x, worldMax.y }, { worldMax.x, worldMax.y});
			//vertical lines
			Renderer2D::DrawLine({ worldMin.x, worldMin.y }, { worldMin.x, worldMax.y});
			Renderer2D::DrawLine({ worldMax.x, worldMin.y }, { worldMax.x, worldMax.y});
		}

	}


	void GameNode::TextCentered(std::string text)
	{
		float win_width = ImGui::GetWindowSize().x;
		float text_width = ImGui::CalcTextSize(text.c_str()).x;

		// calculate the indentation that centers the text on one line, relative
		// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
		float text_indentation = (win_width - text_width) * 0.5f;

		// if text is too long to be drawn on one line, `text_indentation` can
		// become too small or even negative, so we check a minimum indentation
		float min_indentation = 20.0f;
		if (text_indentation <= min_indentation) {
			text_indentation = min_indentation;
		}
		ImGui::SameLine(text_indentation);
		ImGui::PushTextWrapPos(win_width - text_indentation);
		ImGui::TextWrapped(text.c_str());
		ImGui::PopTextWrapPos();
	}


	void GameNode::HandleTickClosure(MergedTickClosure& tc)
	{
		for (int i = 0; i < tc.m_ClientCount; i++)
		{
			HSteamNetConnection clientID;
			tc >> clientID;
			uint32_t inputActionCount;
			tc >> inputActionCount;
			for (int i = 0; i < inputActionCount; i++)
			{
				InputAction IA;
				tc >> IA;
				switch (IA)
				{
				/*case InputAction::Add_Player:
				{
					uint64_t ID;
					tc >> ID;

					glm::ivec2 pixelPos;
					tc >> pixelPos;

					m_World.CreatePlayer(ID, pixelPos);
					break;
				}*/
				case InputAction::PauseGame:
				{
					Pause();
					break;
				}
				case InputAction::ResumeGame:
				{
					Play();
					break;
				}
				case Pyxis::InputAction::Input_Move:
				{
					//PX_TRACE("input action: Input_Move");
					break;
				}
				case Pyxis::InputAction::Input_Place:
				{
					//PX_TRACE("input action: Input_Place");
					bool rigid;
					glm::ivec2 pixelPos;
					uint32_t elementID;
					BrushType brush;
					uint8_t brushSize;
					tc >> rigid >> pixelPos >> elementID >> brush >> brushSize;

					m_World.PaintBrushElement(pixelPos, elementID, brush, brushSize);
					break;
				}
				case Pyxis::InputAction::Input_StepSimulation:
				{
					PX_TRACE("input action: Input_StepSimulation");
					m_World.UpdateWorld();
					break;
					break;
				}
				case InputAction::Input_MousePosition:
				{
					//add new mouse position to data
					glm::vec2 mousePos;
					HSteamNetConnection clientID;
					tc >> mousePos >> clientID;
					m_ClientDataMap[clientID].m_CursorWorldPosition = mousePos;
					break;
				}
				case InputAction::ClearWorld:
				{
					m_World.Clear();
					break;
				}
				default:
				{
					PX_TRACE("input action: default?");
					break;
				}
				}
			}
		}
		
		if (m_World.m_Running)
		{
			m_World.UpdateWorld();
		}
		else
		{
			m_World.UpdateTextures();
		}
	}

	glm::ivec2 GameNode::GetMousePositionImGui()
	{
		///if not using a framebuffer / imgui image, just use Pyxis::Input::GetMousePosition();
		//TODO: Set up ifdef for using imgui? or just stop using imgui... lol
		auto [mx, my] = ImGui::GetMousePos();

		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		return { (int)mx, (int)my };
	}

	glm::vec2 GameNode::GetMousePosWorld()
	{
		glm::vec2 mousePos = Input::GetMousePosition();
		Window& window = Application::Get().GetWindow();
		mousePos.x /= (float)window.GetWidth();
		mousePos.y /= (float)window.GetHeight();
		//from 0->1 to -1 -> 1
		mousePos = (mousePos - 0.5f) * 2.0f;
		mousePos *= (Camera::Main()->GetSize() / 2.0f);
		

		glm::vec4 vec = glm::vec4(mousePos.x, -mousePos.y, 0, 1);
		vec = glm::translate(glm::mat4(1), Camera::Main()->GetPosition()) * vec;
		return vec;
	}

	void GameNode::OnKeyPressedEvent(KeyPressedEvent& event) {
		if (event.GetKeyCode() == PX_KEY_R)
		{
			Physics2D::ResetWorld();
		}
		if (event.GetKeyCode() == PX_KEY_F)
		{
			//Testing rigidbody creation
			glm::vec2 mousePos = GetMousePosWorld();
			glm::ivec2 pixelPos = m_World.WorldToPixel(mousePos);


			std::vector<PixelBodyElement> elements;
			//make a region around the mouse, and turn it into a pixel body
			for (int x = pixelPos.x - m_BrushSize; x < pixelPos.x + m_BrushSize + 1; x++)
			{
				for (int y = pixelPos.y - m_BrushSize; y < pixelPos.y + m_BrushSize + 1; y++)
				{
					glm::ivec2 elementPos = glm::ivec2(x, y);
					Element& e = m_World.GetElement(elementPos);
					if (m_World.m_ElementData[e.m_ID].cell_type == ElementType::solid)
						elements.push_back(PixelBodyElement(m_World.GetElement(elementPos), elementPos));
				}
			}
			Ref<PixelBody2D> newBody;
			if (Pyxis::Input::IsKeyPressed(PX_KEY_LEFT_SHIFT))
			{
				newBody = Instantiate<PixelBody2D>("F-PlacedPixelBody", b2BodyType::b2_staticBody, &m_World, elements, false);
			}
			else
			{
				newBody = Instantiate<PixelBody2D>("F-PlacedPixelBody", b2BodyType::b2_dynamicBody, &m_World, elements, false);
			}
			
			AddChild(newBody);

			SerializeBinary();

		}
		if (event.GetKeyCode() == PX_KEY_SPACE)
		{
			if (m_World.m_Running)
			{
				m_CurrentTickClosure.AddInputAction(InputAction::PauseGame);
			}
			else 
			{
				m_CurrentTickClosure.AddInputAction(InputAction::ResumeGame);
			}
		}
		if (event.GetKeyCode() == PX_KEY_RIGHT)
		{
			m_CurrentTickClosure.AddInputAction(InputAction::Input_StepSimulation);
		}
		if (event.GetKeyCode() == PX_KEY_V)
		{
			Window& window = Application::Get().GetWindow();
			window.SetVSync(!window.IsVSync());
		}
		//switching brushes
		if (event.GetKeyCode() == PX_KEY_Z)
		{
			int type = ((int)m_BrushType) - 1;
			if (type < 0) type = 0;
			m_BrushType = (BrushType)type;
		}
		if (event.GetKeyCode() == PX_KEY_C)
		{
			int type = ((int)m_BrushType) + 1;
			if (type >= (int)BrushType::end) type = (int)BrushType::end - 1;
			m_BrushType = BrushType(type);
		}

	}

	void GameNode::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		//PX_TRACE(event.GetMouseButton());
		if (event.GetMouseButton() == PX_MOUSE_BUTTON_FORWARD) // forward
		{
			m_BrushSize++;
			if (m_BrushSize > 32) m_BrushSize = 32;
		}
		if (event.GetMouseButton() == PX_MOUSE_BUTTON_BACK) // back
		{
			m_BrushSize--;
			if (m_BrushSize < 0.0f) m_BrushSize = 0;
		}
	}

	void GameNode::OnMouseScrolledEvent(MouseScrolledEvent& event)
	{
		if (Input::IsKeyPressed(PX_KEY_LEFT_CONTROL))
		{
			m_BrushSize += event.GetYOffset();
			if (m_BrushSize < 0) m_BrushSize = 0;
			if (m_BrushSize > 32) m_BrushSize = 32;
		}
		
		//do not end event here
	}

	void GameNode::PlayButtonFunc()
	{
		m_CurrentTickClosure.AddInputAction(InputAction::ResumeGame);
	}

	void GameNode::PauseButtonFunc()
	{
		m_CurrentTickClosure.AddInputAction(InputAction::PauseGame);
	}

	void GameNode::Play()
	{
		m_PlayButton->m_Enabled = false;
		m_PauseButton->m_Enabled = true; 
		m_World.m_Running = true;
		//PX_TRACE("Play!");
	}

	void GameNode::Pause()
	{
		m_PlayButton->m_Enabled = true;
		m_PauseButton->m_Enabled = false;
		m_World.m_Running = false;
		//PX_TRACE("Pause!");
	}

	void GameNode::ReturnToMenu()
	{
		auto menu = Instantiate<MenuNode>();
		//m_Parent->AddChild(menu);
		QueueFreeHierarchy();
	}

	/*void GameNode::OnWindowResizeEvent(WindowResizeEvent& event)
	{
		glm::vec2 windowSize = { event.GetWidth(), event.GetHeight() };
		m_Hotbar->ResetLocalTransform();
		m_Hotbar->SetScale((glm::vec3(1) / glm::vec3(windowSize.x / 2.0f, windowSize.y / 2.0f, 1)));
		m_Hotbar->m_Size = { windowSize.x, windowSize.y * 0.1f };
		m_Hotbar->m_TextureScale = 32;
		m_Hotbar->UpdateCanvasTransforms();

	}*/


	void GameNode::PaintBrushHologram()
	{
		glm::ivec2 pixelPos = m_World.WorldToPixel(GetMousePosWorld());

		glm::ivec2 newPos = pixelPos;
		for (int x = -m_BrushSize; x <= m_BrushSize; x++)
		{
			for (int y = -m_BrushSize; y <= m_BrushSize; y++)
			{
				newPos = pixelPos + glm::ivec2(x, y);
				Chunk* chunk;
				glm::ivec2 index;
				switch (m_BrushType)
				{
				case BrushType::circle:
					//limit to circle
					if (std::sqrt((float)(x * x) + (float)(y * y)) >= m_BrushSize) continue;
					break;
				case BrushType::square:
					//don't need to skip any
					break;
				}

				uint32_t color = m_World.m_ElementData[m_SelectedElementIndex].color;

				
				float r = float(color & 0x000000FF) / 255.0f;
				float g = float((color & 0x0000FF00) >> 8) / 255.0f;
				float b = float((color & 0x00FF0000) >> 16) / 255.0f;
				float a = float((color & 0xFF000000) >> 24) / 255.0f;
				glm::vec4 vecColor = glm::vec4(r,g,b,std::fmax(a * 0.5f, 0.25f));
				
				//draw square at that pixel
				float pixelSize = 1.0f / (float)CHUNKSIZE;
				//
				Renderer2D::DrawQuad((glm::vec3(newPos.x, newPos.y, 0) / (float)CHUNKSIZE) + glm::vec3(pixelSize / 2, pixelSize / 2, 0.05f), glm::vec2(pixelSize, pixelSize), vecColor);
				
			}
		}
		
	}


}