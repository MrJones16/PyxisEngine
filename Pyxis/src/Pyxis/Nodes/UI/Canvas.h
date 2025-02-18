#pragma once
#include "UIRect.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that is basically a rect, but with the feature of setting up custom designs / art for
		/// tiling the rect, edges, corners, ect.
		/// </summary>
		class Canvas : public UIRect
		{
		public:

			enum EdgeMode
			{
				Repeat,
				Stretch
			};

			EdgeMode m_EdgeMode = Repeat;

			Ref<Texture2DResource> m_CanvasTextures[9];
			glm::mat4 m_Matrices[9] = { glm::mat4(1) };


			Canvas(const std::string& name = "Canvas") : UIRect(name)
			{

			}

			Canvas(UUID id) : UIRect(id)
			{

			}

			virtual ~Canvas() = default;

			/*
			EdgeMode m_EdgeMode = Repeat;
			Ref<Texture2DResource> m_CanvasTextures[9];
			glm::mat4 m_Matrices[9] = { glm::mat4(1) };
			*/

			//Serialization
			virtual void Serialize(json& j) override
			{
				UIRect::Serialize(j);
				j["Type"] = "Canvas";

				//Add new member variables
				for (int i = 0; i < 9; i++)
				{
					if (m_CanvasTextures[i] != nullptr)
						j["m_CanvasTextures"][std::to_string(i)] = m_CanvasTextures[i]->GetPath();
				}
				

				j["m_EdgeMode"] = m_EdgeMode;
				
			}

			virtual void Deserialize(json& j) override
			{
				UIRect::Deserialize(j);

				//Extract new member variables

				//Add new member variables
				if (j.contains("m_CanvasTextures"))
				{
					for (int i = 0; i < 9; i++)
					{
						if (j["m_CanvasTextures"].contains(std::to_string(i)))
						{
							std::string filepath = "";
							j["m_CanvasTextures"][std::to_string(i)].get_to(filepath);
							m_TextureResource = ResourceManager::Load<Texture2DResource>(filepath);
						}
					}
				}
				
				if (j.contains("m_EdgeMode")) j.at("m_EdgeMode").get_to(m_EdgeMode);
				
				
			}

			/*void UpdateSubTextures()
			{
				///
				/// [ 0 1 2 ]
				/// [ 3 4 5 ]
				/// [ 6 7 8 ]
				/// 
				
				int index = 0;
				for (int y = 0; y < 3; y++)
				{
					for (int x = 0; x < 3; x++)
					{
						m_CanvasTextures[index] = SubTexture2D::CreateFromCoords(m_Texture, { x, y }, { (float)m_Texture->GetWidth() / 3.0f, (float)m_Texture->GetHeight() / 3.0f });
						index++;
					}
				}

			}*/


			/// <summary>
			/// Creates the textures with the desirable settings to be used for the canvas.
			/// 
			/// assumes files are located at folderPath + filePrefix + {1-9} + fileSuffix
			/// 
			/// example: "assets/textures/"   "canvas_"   ".png"
			/// </summary>
			/// <param name="folderPath">Path to the folder containing the 1-9 images</param>
			/// <param name="filePrefix">consistent prefix name for the file, like canvas_</param>
			/// <param name="fileSuffix">file type, like .png</param>
			void CreateTextures(const std::string& folderPath, const std::string& filePrefix, const std::string& fileSuffix)
			{
				Texture::TextureSpecification canvasSpec;
				for (int i = 0; i < 9; i++)
				{
					Texture::TextureSpecification canvasSpec;
					switch (i)
					{
					case 0:
					case 2:
					case 6:
					case 8:
					{
						//corners
						/*canvasSpec.m_TextureModeS = Texture::TextureMode::Stretch;
						canvasSpec.m_TextureModeT = Texture::TextureMode::Stretch;*/
						break;
					}
					case 1:
					{
						//top
						canvasSpec.m_TextureModeS = Texture::TextureMode::Tile;
						break;
					}
					case 3:
					{
						//left
						canvasSpec.m_TextureModeT = Texture::TextureMode::Tile;
						break;
					}
					case 5:
					{
						//right
						canvasSpec.m_TextureModeT = Texture::TextureMode::Tile;
						break;
					}
					case 7:
					{
						//bottom
						canvasSpec.m_TextureModeS = Texture::TextureMode::Tile;
						break;
					}
					case 4:
					{
						//center
					}
					}

					m_CanvasTextures[i] = ResourceManager::Load<Texture2DResource>((folderPath + filePrefix + std::to_string(i + 1) + fileSuffix));
					m_CanvasTextures[i]->m_Texture->SetTextureSpecification(canvasSpec);
					//m_CanvasTextures[i] = Texture2D::Create(folderPath + filePrefix + std::to_string(i + 1) + fileSuffix, canvasSpec);

				}
			}

			virtual void PropagateUpdate() override
			{
				AutoRect();
				UpdateCanvasTransforms();
				UINode::PropagateUpdate();
			}

			virtual void OnInspectorRender() override
			{
				UIRect::OnInspectorRender();
				if (ImGui::TreeNodeEx("Canvas", ImGuiTreeNodeFlags_DefaultOpen))
				{
				


					ImGui::TreePop();
				}
			}


			void UpdateCanvasTransforms()
			{
				/*for (int i = 0; i < 9; i++)
				{
					m_Matrices[i] = glm::mat4(1);
				}*/

				
				//top left
				m_Matrices[0] = glm::translate(glm::mat4(1), { (m_Size.x / -2) + (m_PPU / 2), (m_Size.y / 2) - (m_PPU / 2), 0 });
				m_Matrices[0] = glm::scale(m_Matrices[0], { m_PPU, m_PPU, 1 });

				//top
				m_Matrices[1] = glm::translate(glm::mat4(1), {0, (m_Size.y / 2) - (m_PPU / 2), 0});
				m_Matrices[1] = glm::scale(m_Matrices[1], { m_Size.x - (m_PPU * 2), m_PPU, 1 });

				//top right
				m_Matrices[2] = glm::translate(glm::mat4(1), { (m_Size.x / 2) - (m_PPU / 2), (m_Size.y / 2) - (m_PPU / 2), 0 });
				m_Matrices[2] = glm::scale(m_Matrices[2], { m_PPU, m_PPU, 1 });

				//left
				m_Matrices[3] = glm::translate(glm::mat4(1), { (m_Size.x / -2) + (m_PPU / 2), 0, 0 });
				m_Matrices[3] = glm::scale(m_Matrices[3], { m_PPU, m_Size.y - (m_PPU * 2), 1 });

				//center
				m_Matrices[4] = glm::scale(glm::mat4(1.0f), { m_Size.x - (m_PPU * 2), m_Size.y - (m_PPU * 2), 1 });
				
				//right
				m_Matrices[5] = glm::translate(glm::mat4(1), { (m_Size.x / 2) - (m_PPU / 2), 0, 0 });
				m_Matrices[5] = glm::scale(m_Matrices[5], { m_PPU, m_Size.y - (m_PPU * 2), 1 });

				//Bottom Left
				m_Matrices[6] = glm::translate(glm::mat4(1), { (m_Size.x / -2) + (m_PPU / 2), (m_Size.y / -2) + (m_PPU / 2), 0 });
				m_Matrices[6] = glm::scale(m_Matrices[6], { m_PPU, m_PPU, 1 });
				
				//bottom
				m_Matrices[7] = glm::translate(glm::mat4(1), { 0, (m_Size.y / -2) + (m_PPU / 2), 0 });
				m_Matrices[7] = glm::scale(m_Matrices[7], { m_Size.x - (m_PPU * 2), m_PPU, 1 });

				//Bottom Right
				m_Matrices[8] = glm::translate(glm::mat4(1), { (m_Size.x / 2) - (m_PPU / 2), (m_Size.y / -2) + (m_PPU / 2), 0 });
				m_Matrices[8] = glm::scale(m_Matrices[8], { m_PPU, m_PPU, 1 });

			}

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_CanvasTextures[0] != nullptr)
					{
						
						//first, lets render the texture for the center?
						glm::mat4 worldTransform = GetWorldTransform();
						//
						//UpdateCanvasTransforms();
						//now, we should render the outside edges, 
						for (int i = 0; i < 9; i++)
						{
							Renderer2D::DrawQuadEntity(worldTransform * m_Matrices[i], m_CanvasTextures[i]->m_Texture, GetUUID());
						}
					}
					else if (m_TextureResource != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, GetUUID());
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetUUID());
					}

				}

			}

		};

		REGISTER_SERIALIZABLE_NODE(Canvas);

	}
}