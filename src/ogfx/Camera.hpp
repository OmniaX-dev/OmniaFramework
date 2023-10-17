#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include <ostd/Geometry.hpp>
#include <ostd/BaseObject.hpp>
#include <vendor/glm/glm.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove from header

	class Window;
	class FirstPersonCamera : BaseObject
	{
		public:
			inline FirstPersonCamera(void) { invalidate(); }
			inline FirstPersonCamera(Window& window, bool invert_y = false) { create(window, invert_y); }
			FirstPersonCamera& create(Window& window, bool invert_y = false);
			void mouseMoved(float x, float y);
			void updateViewMatrix(void);
			void updateProjectionMatrix(float fov = 0.0f, float near = 0.0f, float far = 0.0f);
			inline glm::mat4& getViewMat(void) { return m_viewMat; }
			inline glm::mat4& getProjMat(void) { return m_perspMat; }
			void update(void);
			void KeyPressed(uint32_t key);
			void keyReleased(uint32_t key);
			
			inline ostd::Vec3 getPosition(void) { return { m_position.x, m_position.y, m_position.z }; }
			inline ostd::Vec3 getFront(void) { return { m_front.x, m_front.y, m_front.z }; }

		private:
			glm::vec3 m_position { 0.0f, 0.0f, 10.0f };
			glm::vec3 m_front { 0.0f, 0.0f, -1.0f };
			glm::vec3 m_up { 0.0f, 1.0f, 0.0f };

			float m_yaw { -90.0f };
			float m_pitch { 0.0f };

			float m_moveSpeed { 0.4f };
			float m_moveSensitivity { 0.02f };

			bool m_invert_y { false };
			bool m_reset_mouse { true };

			bool m_move_keys[6] { false, false, false, false, false, false };

			glm::mat4 m_viewMat { 1.0f };
			glm::mat4 m_perspMat { 1.0f };
		
			Window* m_window { nullptr };

		public:
			inline static constexpr uint32_t KeyForward = 0;
			inline static constexpr uint32_t KeyBackwards = 2;
			inline static constexpr uint32_t KeyLeft = 1;
			inline static constexpr uint32_t KeyRight = 3;
			inline static constexpr uint32_t KeyUp = 4;
			inline static constexpr uint32_t KeyDown = 5;
	};
	
	class OrthoCamera : public BaseObject
    {
        public:
            inline OrthoCamera(void) { invalidate(); }
            //inline OrthoCamera(float left, float right, float bottom, float top, float near = -1.0f, float far = 1.0f) { create(left, right, bottom, top, near, far); }
            OrthoCamera(float left, float right, float bottom, float top, float near = -1.0f, float far = 1.0f);
			OrthoCamera& createDefault(float windowWidth, float windowHeight, bool soft = false);

			void handleSignal(ostd::tSignal& signal) override;

            inline const glm::vec3& getPosition(void) const { return m_position; }
            inline void setPosition(const glm::vec3& pos) { m_position = pos; __update_view_proj_mat(); }

            inline float getRotation(void) const { return m_rotation; }
            inline void setRotation(float rot) { m_rotation = rot; __update_view_proj_mat(); }

            inline const glm::mat4& getProjectionMatrix(void) const { return m_projection; }
            inline const glm::mat4& getViewMatrix(void) const { return m_view; }
            inline const glm::mat4& getViewProjectionMatrix(void) const { return m_viewProj; }

            String toString(void) const override;

        private:
            void __update_view_proj_mat(void);
		    OrthoCamera& create(float left, float ight, float bottom, float top, float near = -1.0f, float far = 1.0f, bool soft = false);

        private:
            glm::mat4 m_projection { 1.0f };
            glm::mat4 m_view { 1.0f };
            glm::mat4 m_viewProj { 1.0f };
            glm::vec3 m_position { 0.0f, 0.0f, 0.0f };
            float m_rotation { 0.0f };
    };

    typedef OrthoCamera Camera2D;
}

#endif
