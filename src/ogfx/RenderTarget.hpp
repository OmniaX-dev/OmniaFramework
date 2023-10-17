#ifndef __RENDER_TARGET_HPP__
#define __RENDER_TARGET_HPP__

#include <ostd/BaseObject.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Types.hpp>
#include <ostd/Defines.hpp>

namespace ogfx
{
	class RenderTarget : public ostd::BaseObject
    {
        public:
            inline RenderTarget(void) { invalidate(); }
            inline RenderTarget(int32_t width, int32_t height) { create(width, height); }
            virtual ~RenderTarget(void);
			void destroy(void);
            RenderTarget& create(int32_t width, int32_t height, bool soft = false);

            inline uint32_t getOpenGLFrameBufferID(void) const { return static_cast<uint32_t>(getID()); }
            inline uint32_t getOpenGLRenderBufferID(void) const { return m_rbo_gl_id; }
			inline ostd::ResourceID getResourceID(void) const { return m_texture; }

            void bind(void) const;
            void unbind(void) const;
            void bindScreenTexture(void) const;

            void handleSignal(ostd::tSignal& signal) override;

			inline ostd::Vec2 getSize(void) const { return { (float)m_width, (float)m_height }; }

        private:
			ostd::ResourceID m_texture;
            uint32_t m_rbo_gl_id;
            int32_t m_width;
            int32_t m_height;
			bool m_signalConnected { false };

		public:
			inline static constexpr int32_t ERR_FAILED_TO_CREATE_FRAMEBUFFER = OX_RENDERTARGET_ERR_MASK + 0x0001;
    };

	typedef RenderTarget RenderTexture;
}

#endif
