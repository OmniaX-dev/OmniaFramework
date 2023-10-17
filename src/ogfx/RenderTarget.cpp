#include "RenderTarget.hpp"
#include "GLBuffers.hpp"
#include "Errors.hpp"
#include "ResourceManager.hpp"
#include "Signals.hpp"

namespace ogfx
{
	using namespace ostd;

	RenderTarget::~RenderTarget(void)
	{
		destroy();
	}

	void RenderTarget::destroy(void)
	{
		uint32_t id = getOpenGLFrameBufferID();
		GLCall(glDeleteRenderbuffers(1, &m_rbo_gl_id));
		GLCall(glDeleteFramebuffers(1, &id));
		ResourceManager::destroyResource(m_texture, tResourceType::Texture);
		setID(id);
		invalidate();
	}

    RenderTarget& RenderTarget::create(int32_t width, int32_t height, bool soft)
	{
		m_texture = ResourceManager::newTexture(width, height);
		m_width = width;
		m_height = height;

		uint32_t gl_id;
		GLCall(glGenFramebuffers(1, &gl_id));
		setID(gl_id);
		bind();
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  ResourceManager::getTexture(m_texture).getOpenGLID(), 0));
		GLCall(glGenRenderbuffers(1, &m_rbo_gl_id));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_gl_id));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height));
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo_gl_id));
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			ErrorHandler::pushError(RenderTarget::ERR_FAILED_TO_CREATE_FRAMEBUFFER);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("%s", err_str.c_str());
			unbind();
			destroy();
			return *this;
		}
		unbind();

		enableSignals();
		if (!soft)
			connectSignal(tBuiltinSignals::WindowResized);

		setTypeName("ogfx::RenderTarget");
		validate();

		return *this;
	}

	void RenderTarget::bind(void) const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, getOpenGLFrameBufferID());
	}

	void RenderTarget::unbind(void) const
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void RenderTarget::bindScreenTexture(void) const
	{
		ResourceManager::getTexture(m_texture).bind();
	}

	void RenderTarget::handleSignal(tSignal& signal)
	{
		if (signal.ID == tBuiltinSignals::WindowResized)
		{
			WindowSizeObj wsobj = static_cast<WindowSizeObj&>(signal.userData);
			unbind();
			destroy();
			create(wsobj.width, wsobj.height, true);
		}
	}
}
