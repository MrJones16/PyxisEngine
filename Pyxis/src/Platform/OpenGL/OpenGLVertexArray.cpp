#include "pxpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Pyxis
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Pyxis::ShaderDataType::Bool:     return GL_BOOL;
		case Pyxis::ShaderDataType::Float:    return GL_FLOAT;
		case Pyxis::ShaderDataType::Float2:   return GL_FLOAT;
		case Pyxis::ShaderDataType::Float3:   return GL_FLOAT;
		case Pyxis::ShaderDataType::Float4:   return GL_FLOAT;
		case Pyxis::ShaderDataType::Mat2:     return GL_FLOAT;
		case Pyxis::ShaderDataType::Mat3:     return GL_FLOAT;
		case Pyxis::ShaderDataType::Mat4:     return GL_FLOAT;
		case Pyxis::ShaderDataType::Int:      return GL_INT;
		case Pyxis::ShaderDataType::Int2:     return GL_INT;
		case Pyxis::ShaderDataType::Int3:     return GL_INT;
		case Pyxis::ShaderDataType::Int4:     return GL_INT;
		case Pyxis::ShaderDataType::Uint:     return GL_UNSIGNED_INT;
		}

		PX_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		PX_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				case ShaderDataType::Mat2:
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)element.Offset); 
					index++;
					break;
				}
				case ShaderDataType::Bool:
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Uint:
				{
					glEnableVertexAttribArray(index);
					glVertexAttribIPointer(index,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.Type),
						layout.GetStride(),
						(const void*)element.Offset);
					index++;
					break;
				}
				case ShaderDataType::None:
				default:
				{
					break;
				}
			}
			
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}