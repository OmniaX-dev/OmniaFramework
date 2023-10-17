#include "GLBuffers.hpp"
#include "Errors.hpp"
//TODO: Implement Errors

namespace ogfx
{
	using namespace ostd;

	VertexBuffer::~VertexBuffer(void)
	{
		if (isInvalid()) return;
		uint32_t gl_id = getOpenGLID();
		GLCall(glDeleteBuffers(1, &gl_id));
	}

	VertexBuffer& VertexBuffer::create(const void* data, uint32_t byte_size, bool dynamic)
	{
		uint32_t gl_id;
		GLCall(glGenBuffers(1, &gl_id));
		setID(gl_id);
		bind();
		GLCall(glBufferData(GL_ARRAY_BUFFER, byte_size, data, (dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW)));
		unbind();
		setTypeName("ox::VertexBuffer");
		validate();
		return *this;
	}

	void VertexBuffer::bind(void) const
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, getOpenGLID()));
	}

	void VertexBuffer::unbind(void) const
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}



	ElementBuffer::~ElementBuffer(void)
	{
		if (isInvalid()) return;
		uint32_t gl_id = getOpenGLID();
		GLCall(glDeleteBuffers(1, &gl_id));
	}

	ElementBuffer& ElementBuffer::create(const void* data, uint32_t elem_count)
	{
		m_elem_count = elem_count;
		uint32_t gl_id;
		GLCall(glGenBuffers(1, &gl_id));
		setID(gl_id);
		bind();
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_elem_count * sizeof(uint32_t), data, GL_STATIC_DRAW));
		unbind();
		setTypeName("ox::ElementBuffer");
		validate();
		return *this;
	}

	void ElementBuffer::bind(void) const
	{
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, getOpenGLID()));
	}

	void ElementBuffer::unbind(void) const
	{
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}



	uint32_t tVertexBufferElem::gl_typeToSize(uint32_t type)
	{
		switch (type)
		{
			case GL_FLOAT:			return sizeof(GLfloat);
			case GL_UNSIGNED_INT:	return sizeof(GLuint);
			case GL_INT:			return sizeof(GLint);
			case GL_BYTE:			return sizeof(GLbyte);
			case GL_UNSIGNED_BYTE:	return sizeof(GLubyte);
			default: break;
		}
		OX_WARN("Unkwnown type in tVertexBufferElem");
		return 0;
	}




	VertexArray::VertexArray(bool init)
	{
		if (!init) return;
		create();
	}

	VertexArray::~VertexArray(void)
	{
		if (isInvalid()) return;
		uint32_t gl_id = getOpenGLID();
		GLCall(glDeleteVertexArrays(1, &gl_id));
	}

	VertexArray& VertexArray::create(void)
	{
		uint32_t gl_id;
		GLCall(glGenVertexArrays(1, &gl_id));
		setID(gl_id);
		setTypeName("ox::VertexArray");
		return *this;
	}

	uint32_t VertexArray::addBuffer(VertexBuffer& vbo, const VertexBufferLayout& layout)
	{
		bind();
		vbo.bind();
		const auto& elements = layout.getElements();
		uint64_t offset = 0;
		for (uint32_t i = 0; i < elements.size(); i++)
		{
			const auto& elem = elements[i];
			GLCall(glEnableVertexAttribArray(i));
			GLCall(glVertexAttribPointer(i, elem.count, elem.type, elem.normalized, layout.getStride(), (const void*)offset));
			offset += elem.count * tVertexBufferElem::gl_typeToSize(elem.type);
		}
		unbind();
		vbo.unbind();
		m_vertexBuffers.push_back(&vbo);
		return m_vertexBuffers.size() - 1;
	}

	void VertexArray::setElementBuffer(const ElementBuffer& ebo)
	{
		if (!ebo.isValid())
		{
			OX_WARN("Invalid ElementBuffer object passed in VertexArray.");
			return;
		}
		bind();
		ebo.bind();
		unbind();
		ebo.unbind();
		validate();
	}
	
	void VertexArray::bind(void) const
	{
		GLCall(glBindVertexArray(getOpenGLID()));
	}

	void VertexArray::unbind(void) const
	{
		GLCall(glBindVertexArray(0));
	}

	VertexBuffer& VertexArray::getVerteBuffer(uint32_t internal_index)
	{
		if (internal_index >= m_vertexBuffers.size()) return (VertexBuffer&)BaseObject::InvalidRef();
		return *(m_vertexBuffers[internal_index]);
	}

	uint32_t VertexArray::getElementCount(void) const
	{
		return m_elem_count;
	}
}
