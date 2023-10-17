#ifndef __DATA_STRUCTURES_HPP__
#define __DATA_STRUCTURES_HPP__

#include <vendor/glm/matrix.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Color.hpp>
#include <ogfx/GLBuffers.hpp>
#include <vendor/glm/gtc/matrix_transform.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove from header

    struct tVertex
    {
        Vec3 position;
        Color::FloatCol color;
        Vec2 texCoords;
		Vec3 normal;

        inline static VertexBufferLayout getVertexBufferLayout(void)
        {
            VertexBufferLayout layout;
            layout.push<float>(3);
            layout.push<float>(4);
            layout.push<float>(2);
            layout.push<float>(3);
            return layout;
        }
    };
    struct tVertex2D
    {
        Vec3 position;
        Color::FloatCol color;
        Vec2 texCoords;
        float texIndex { 0.0f };

        inline static VertexBufferLayout getVertexBufferLayout(void)
        {
            VertexBufferLayout layout;
            layout.push<float>(3);
            layout.push<float>(4);
            layout.push<float>(2);
            layout.push<float>(1);
            return layout;
        }
    };
	
	struct tPerspective
	{
		float FOV { 45.0f };
		float windowWidth { 0 };
		float windowHeight { 0 };
		float nearPlane { 0.1f };
		float farPlane { 300.0f };

		inline tPerspective(int32_t win_w, float win_h) { windowWidth = win_w; windowHeight = win_h; update(); }
		inline tPerspective(float fov, float win_w, float win_h, float near, float far)
		{
			FOV = fov;
			windowWidth = win_w;
			windowHeight = win_h;
			nearPlane = near;
			farPlane = far;
			update();
		}
		inline void update(void) { m_matrix = glm::perspective(FOV, getAspectRatio(), nearPlane, farPlane); }
		inline float getAspectRatio(void) { return windowWidth / windowHeight; }
		inline const glm::mat4& getMatrix(void) { return m_matrix; }

		private:
			glm::mat4 m_matrix;
	};

    class Transform2D
    {
        private:
            float m_rotation { 0.0f };
            Vec2 m_translation { 0.0f, 0.0f };
            Vec2 m_scale { 1.0f, 1.0f };
            bool m_centeredOrigin { false };
            Vec2 m_baseSize { 0.0f, 0.0f };

            bool m_applied { false };
            glm::mat4 m_matrix;
            std::vector<Vec2> m_vertices;

        public:
			inline Transform2D(void) { apply(); }

            inline Transform2D& resetRotation(float newValue = 0.0f) { m_applied = (m_rotation == newValue && m_applied); m_rotation = newValue; return *this; }
            inline Transform2D& resetTranslation(Vec2 newValue = { 0.0f, 0.0f }) { m_applied = (m_translation == newValue && m_applied); m_translation = newValue; return *this; }
            inline Transform2D& resetScale(Vec2 newValue = { 1.0f, 1.0f }) { m_applied = (m_scale == newValue && m_applied); m_scale = newValue; return *this; }
            inline Transform2D& reset(float _rotation = 0.0f, Vec2 _translation = { 0.0f, 0.0f }, Vec2 _scale = { 1.0f,1.0f }) { resetRotation(_rotation); resetTranslation(_translation); return resetScale(_scale); }

            inline Transform2D& rotate(float value) { m_applied = (value == 0.0f && m_applied); m_rotation += value; return *this; }
            inline Transform2D& translate(Vec2 value) { m_applied = (value == Vec2(0.0f, 0.0f) && m_applied); m_translation += value; return *this; }
            inline Transform2D& scale(Vec2 value) { m_applied = (value == Vec2(0.0f, 0.0f) && m_applied); m_scale += value; return *this; }

            inline float getRotation(void) const { return m_rotation; }
            inline Vec2 getTranslation(void) const { return m_translation; }
            inline Vec2 getScale(void) const { return m_scale; }
            inline bool isOriginCentered(void) const { return m_centeredOrigin; }
            inline bool isApplied(void) const { return m_applied; }
            inline glm::mat4 getMatrix(void) const { return m_matrix; }
            inline std::vector<Vec2> getVertices(void) const { return m_vertices; }
            inline Vec2 getBaseSize(void) const { return m_baseSize; }

            inline Transform2D& setOriginCentered(bool b = true) { m_applied = (m_centeredOrigin == b && m_applied); m_centeredOrigin = b; return *this; }
            inline Transform2D& setBaseSize(Vec2 baseSize) { m_applied = (m_baseSize == baseSize && m_applied); m_baseSize = baseSize; return *this; }

            inline Transform2D& apply(void)
            {
                if (m_applied) return *this;

                glm::vec4 npos(0.0f, 0.0f, 0.0f, 1.0f);
                glm::mat4 model = glm::translate(glm::mat4(1.0f), { m_translation.x, m_translation.y, 0.0f });
                model = glm::rotate(model, DEG_TO_RAD(m_rotation), { 0.0f, 0.0f, 1.0f });
                model = glm::scale(model, { m_scale.x, m_scale.y, 1.0f });
                
                m_vertices.clear();
                if (m_centeredOrigin)
                {
                    npos = { -(m_baseSize.x / 2.0f), -(m_baseSize.y / 2.0f), 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                    npos = { m_baseSize.x / 2.0f, -(m_baseSize.y / 2.0f), 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                    npos = { m_baseSize.x / 2.0f, m_baseSize.y / 2.0f, 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                    npos = { -(m_baseSize.x / 2.0f), m_baseSize.y / 2.0f, 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                }
                else
                {
                    npos = { 0.0f, 0.0f, 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                    npos = { m_baseSize.x, 0.0f, 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                    npos = { m_baseSize.x, m_baseSize.y, 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                    npos = { 0.0f, m_baseSize.y, 0.0f, 1.0f };
                    npos = model * npos;
                    m_vertices.push_back({ npos.x, npos.y });
                }
                
                m_applied = true;
                return *this;
            }
    };

	class Transform3D
    {
        private:
            Vec3 m_rotation { 0.0f, 0.0f, 0.0f };
            Vec3 m_translation { 0.0f, 0.0f, 0.0f };
            Vec3 m_scale { 1.0f, 1.0f, 1.0f };

			bool m_updateNormalMatrix { true };
            bool m_applied { true };
            glm::mat4 m_matrix;
			glm::mat4 m_normalMatrix;

        public:
			inline Transform3D(void) { m_matrix = glm::mat4(1.0f); m_normalMatrix = m_matrix; }

            inline Transform3D& loadIdentity(void) { m_matrix = glm::mat4(1.0f); m_applied = true; m_rotation = { 0.0f, 0.0f, 0.0f }; m_translation = { 0.0f, 0.0f, 0.0f }; m_scale = { 1.0f, 1.0f, 1.0f }; m_normalMatrix = m_matrix; m_updateNormalMatrix = false; m_applied = true; return *this; }

            inline Transform3D& rotate(Vec3 value) { m_applied = (value == Vec3(0.0f, 0.0f, 0.0f) && m_applied); m_updateNormalMatrix = !m_applied; m_rotation += value; return *this; }
            inline Transform3D& translate(Vec3 value) { m_applied = (value == Vec3(0.0f, 0.0f, 0.0f) && m_applied); m_translation += value; return *this; }
            inline Transform3D& scale(Vec3 value) { m_applied = (value == Vec3(0.0f, 0.0f, 0.0f) && m_applied); m_updateNormalMatrix = !m_applied; m_scale += value; return *this; }

            inline Vec3 getRotation(void) const { return m_rotation; }
            inline Vec3 getTranslation(void) const { return m_translation; }
            inline Vec3 getScale(void) const { return m_scale; }
            inline bool isApplied(void) const { return m_applied; }
            inline glm::mat4 getMatrix(void) const { return m_matrix; }
			inline glm::mat4 getNormalMatrix(void) const { return m_normalMatrix; }

            inline Transform3D& apply(void)
            {
                if (m_applied) return *this;

                m_matrix = glm::translate(glm::mat4(1.0f), { m_translation.x, m_translation.y, m_translation.z });
                m_matrix = glm::scale(m_matrix, { m_scale.x, m_scale.y, m_scale.z });
                m_matrix = glm::rotate(m_matrix, DEG_TO_RAD(m_rotation.z), { 0.0f, 0.0f, 1.0f });
                m_matrix = glm::rotate(m_matrix, DEG_TO_RAD(m_rotation.y), { 0.0f, 1.0f, 0.0f });
                m_matrix = glm::rotate(m_matrix, DEG_TO_RAD(m_rotation.x), { 1.0f, 0.0f, 0.0f });

				if (m_updateNormalMatrix)
				{
					m_normalMatrix = glm::transpose(glm::inverse(m_matrix));
					m_updateNormalMatrix = false;
				}
                
                m_applied = true;
                return *this;
            }
    };

	struct tContextSettings
	{
		struct tGLProfile
		{
			inline static constexpr uint8_t Core = 0;
			inline static constexpr uint8_t Compatibility = 1;
		};

		uint8_t depthBits { 24 };
		uint8_t stencilBits { 8 };
		uint8_t antiAliasLevel { 0 }; //TODO: Add "enums"
		uint8_t GLMajorVersion = { 4 };
		uint8_t GLMinorVersion { 2 };
		uint8_t GLProfile { tGLProfile::Core };
	};
    
}
 
 #endif
