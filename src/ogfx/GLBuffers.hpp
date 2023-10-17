#ifndef __GL_BUFFERS_HPP__
#define __GL_BUFFERS_HPP__

#include <vector>
#include <ostd/BaseObject.hpp>
#include <vendor/GLAD/glad/glad.h>
#include <ostd/Defines.hpp>
#include <ostd/Logger.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove this from header

    class VertexBuffer : public BaseObject
	{
		public:
			inline VertexBuffer(void) { invalidate(); }
			inline VertexBuffer(const void* data, uint32_t byte_size, bool dynamic = false) { create(data, byte_size, dynamic); }
			~VertexBuffer(void);
			VertexBuffer& create(const void* data, uint32_t byte_size, bool dynamic = false);
			void bind(void) const;
			void unbind(void) const;
			inline uint32_t getOpenGLID(void) const { return static_cast<uint32_t>(getID()); }
	};

	class ElementBuffer : public BaseObject
	{
		public:
			inline ElementBuffer(void) { invalidate(); }
			inline ElementBuffer(const uint32_t* data, uint32_t elem_count) { create(data, elem_count); }
			~ElementBuffer(void);
			ElementBuffer& create(const void* data, uint32_t elem_count);
			void bind(void) const;
			void unbind(void) const;
			inline uint32_t getElementCount(void) const { return m_elem_count; }
			inline uint32_t getOpenGLID(void) const { return static_cast<uint32_t>(getID()); }

		private:
			uint32_t m_elem_count { 0 };
	};

	struct tVertexBufferElem
	{
		uint32_t type;
		uint32_t count;
		int32_t normalized;

		static uint32_t gl_typeToSize(uint32_t type);
	};

	class VertexBufferLayout
	{
		public:
			inline VertexBufferLayout(void) : m_stride(0) {  }
			template<typename T>
			inline void push(uint32_t count)
			{
				if constexpr(std::is_same_v<T, float>)
				{
					m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
					m_stride += count * tVertexBufferElem::gl_typeToSize(GL_FLOAT);
				}
				else if constexpr(std::is_same_v<T, uint32_t>)
				{
					m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
					m_stride += count * tVertexBufferElem::gl_typeToSize(GL_UNSIGNED_INT);
				}
				else if constexpr(std::is_same_v<T, int32_t>)
				{
					m_elements.push_back({ GL_INT, count, GL_FALSE });
					m_stride += count * tVertexBufferElem::gl_typeToSize(GL_INT);
				}
				else if constexpr(std::is_same_v<T, uint8_t>)
				{
					m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
					m_stride += count * tVertexBufferElem::gl_typeToSize(GL_UNSIGNED_BYTE);
				}
				else if constexpr(std::is_same_v<T, int8_t>)
				{
					m_elements.push_back({ GL_BYTE, count, GL_TRUE });
					m_stride += count * tVertexBufferElem::gl_typeToSize(GL_BYTE);
				}
				else
				{
					OX_WARN("Invalid template parameter in ox::VertexBufferLayout");
					return;
				}
			}
			inline uint32_t getStride(void) const { return m_stride; }
			inline const std::vector<tVertexBufferElem>& getElements(void) const { return m_elements; }

		private:
			std::vector<tVertexBufferElem> m_elements;
			uint32_t m_stride;
	};

	class VertexArray : public BaseObject
	{
		public:
			VertexArray(bool init = true);
			~VertexArray(void);
			VertexArray& create(void);
			uint32_t addBuffer(VertexBuffer& vbo, const VertexBufferLayout& layout);
			void setElementBuffer(const ElementBuffer& ebo);
			void bind(void) const;
			void unbind(void) const;
			uint32_t getElementCount(void) const;
			VertexBuffer& getVerteBuffer(uint32_t internal_index = 0);
			inline uint32_t getOpenGLID(void) const { return static_cast<uint32_t>(getID()); }

		private:
			uint32_t m_elem_count;
			std::vector<VertexBuffer*> m_vertexBuffers;

		public:
			inline static constexpr int32_t ERR_NO_EBO_SET = OX_GLBUFFERS_ERR_MASK + 0x0001;
	};
}

#endif
