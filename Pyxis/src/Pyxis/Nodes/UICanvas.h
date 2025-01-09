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
		class UICanvas : public UIRect
		{
		public:

			enum EdgeMode
			{
				Repeat,
				Stretch
			};

			EdgeMode m_EdgeMode = Repeat;

			Ref<Texture2D> m_CanvasTextures[9];
			glm::mat4 m_Matrices[9] = { glm::mat4(1) };

			float m_TextureScale = 1.0f;


			UICanvas(const std::string& name = "UICanvas") : UIRect(name)
			{

			}

			UICanvas(Ref<Texture2D> texture, const std::string& name = "UICanvas") : UIRect(texture, name)
			{

			}

			UICanvas(const glm::vec4& color, const std::string& name = "UICanvas") : UIRect(color, name)
			{

			}

			virtual ~UICanvas() = default;

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

					m_CanvasTextures[i] = Texture2D::Create(folderPath + filePrefix + std::to_string(i + 1) + fileSuffix, canvasSpec);

				}
			}

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Container", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//Size
					ImGui::DragFloat2("Size", glm::value_ptr(m_Size));

					//texture scale
					ImGui::DragFloat("Texture Scaling", &m_TextureScale);

					/*if (ImGui::Button("SetSubTextures"))
					{
						UpdateSubTextures();
					}*/
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
				m_Matrices[0] = glm::translate(glm::mat4(1), { (m_Size.x / -2) + (m_TextureScale / 2), (m_Size.y / 2) - (m_TextureScale / 2), 0 });
				m_Matrices[0] = glm::scale(m_Matrices[0], { m_TextureScale, m_TextureScale, 1 });

				//top
				m_Matrices[1] = glm::translate(glm::mat4(1), {0, (m_Size.y / 2) - (m_TextureScale / 2), 0});
				m_Matrices[1] = glm::scale(m_Matrices[1], { m_Size.x - (m_TextureScale * 2), m_TextureScale, 1 });

				//top right
				m_Matrices[2] = glm::translate(glm::mat4(1), { (m_Size.x / 2) - (m_TextureScale / 2), (m_Size.y / 2) - (m_TextureScale / 2), 0 });
				m_Matrices[2] = glm::scale(m_Matrices[2], { m_TextureScale, m_TextureScale, 1 });

				//left
				m_Matrices[3] = glm::translate(glm::mat4(1), { (m_Size.x / -2) + (m_TextureScale / 2), 0, 0 });
				m_Matrices[3] = glm::scale(m_Matrices[3], { m_TextureScale, m_Size.y - (m_TextureScale * 2), 1 });

				//center
				m_Matrices[4] = glm::scale(glm::mat4(1.0f), { m_Size.x - (m_TextureScale * 2), m_Size.y - (m_TextureScale * 2), 1 });
				
				//right
				m_Matrices[5] = glm::translate(glm::mat4(1), { (m_Size.x / 2) - (m_TextureScale / 2), 0, 0 });
				m_Matrices[5] = glm::scale(m_Matrices[5], { m_TextureScale, m_Size.y - (m_TextureScale * 2), 1 });

				//Bottom Left
				m_Matrices[6] = glm::translate(glm::mat4(1), { (m_Size.x / -2) + (m_TextureScale / 2), (m_Size.y / -2) + (m_TextureScale / 2), 0 });
				m_Matrices[6] = glm::scale(m_Matrices[6], { m_TextureScale, m_TextureScale, 1 });
				
				//bottom
				m_Matrices[7] = glm::translate(glm::mat4(1), { 0, (m_Size.y / -2) + (m_TextureScale / 2), 0 });
				m_Matrices[7] = glm::scale(m_Matrices[7], { m_Size.x - (m_TextureScale * 2), m_TextureScale, 1 });

				//Bottom Right
				m_Matrices[8] = glm::translate(glm::mat4(1), { (m_Size.x / 2) - (m_TextureScale / 2), (m_Size.y / -2) + (m_TextureScale / 2), 0 });
				m_Matrices[8] = glm::scale(m_Matrices[8], { m_TextureScale, m_TextureScale, 1 });

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
						UpdateCanvasTransforms();
						//now, we should render the outside edges, 
						for (int i = 0; i < 9; i++)
						{
							Renderer2D::DrawQuadEntity(worldTransform * m_Matrices[i], m_CanvasTextures[i], GetID());
						}
					}
					else if (m_Texture != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Texture, GetID());
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetID());
					}

				}

			}

		};

	}
}