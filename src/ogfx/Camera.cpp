#include "Camera.hpp"
#include "DataStructures.hpp"

#include "Window.hpp"
#include "Signals.hpp"

#include <ostd/Utils.hpp>

namespace ogfx
{
	using namespace ostd;

	FirstPersonCamera& FirstPersonCamera::create(Window &window, bool invert_y)
	{
		m_window = &window;
		m_invert_y = invert_y;
		
		updateProjectionMatrix();

		setTypeName("ogfx::FirstPersonCamera");
		validate();

		updateViewMatrix();
		return *this;
	}

	void FirstPersonCamera::mouseMoved(float x, float y)
	{
		if (isInvalid()) return;

		m_reset_mouse = true;
		float xpos = x - (m_window->getSize().x / 2.0f); 
		float ypos = y - (m_window->getSize().y / 2.0f);

		xpos *= m_moveSensitivity;
		ypos *= m_moveSensitivity * (m_invert_y ? 1.0f : -1.0f);

		m_yaw += xpos;
		m_pitch += ypos;

		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front.y = sin(glm::radians(m_pitch));
		front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_front = glm::normalize(front);
	}

	void FirstPersonCamera::updateViewMatrix(void)
	{
		if (isInvalid()) return;

		m_viewMat = glm::lookAt(m_position, m_position + m_front, m_up);
	}

	void FirstPersonCamera::updateProjectionMatrix(float fov, float near, float far)
	{
		tPerspective persp(m_window->getSize().x, m_window->getSize().y);
		if (fov != 0.0f) persp.FOV = fov;
		if (near != 0.0f) persp.nearPlane = near;
		if (far != 0.0f) persp.farPlane = far;
		persp.update();
		m_perspMat = persp.getMatrix();
	}

	void FirstPersonCamera::update(void)
	{
		if (isInvalid()) return;

		if (m_reset_mouse)
		{
			m_window->centerMouse();
			m_reset_mouse = false;
		}

		glm::vec3 absolute_front = m_front;
		absolute_front.y = 0.0f;

		if (m_move_keys[KeyForward])
			m_position += m_moveSpeed * absolute_front;
		if (m_move_keys[KeyLeft])
			m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_moveSpeed;
		if (m_move_keys[KeyBackwards])
			m_position -= m_moveSpeed * absolute_front;
		if (m_move_keys[KeyRight])
			m_position += glm::normalize(glm::cross(m_front, m_up)) * m_moveSpeed;
		if (m_move_keys[KeyUp])
			m_position += m_moveSpeed * m_up;
		if (m_move_keys[KeyDown])
			m_position -= m_moveSpeed * m_up;

		updateViewMatrix();
	}

	void FirstPersonCamera::KeyPressed(uint32_t key)
	{
		if (isInvalid()) return;
		switch (key)
		{
			case KeyForward:
			case KeyLeft:
			case KeyBackwards:
			case KeyRight:
			case KeyUp:
			case KeyDown:
				break;
			default: return;
		}
		m_move_keys[key] = true;
	}

	void FirstPersonCamera::keyReleased(uint32_t key)
	{
		if (isInvalid()) return;
		switch (key)
		{
			case KeyForward:
			case KeyLeft:
			case KeyBackwards:
			case KeyRight:
			case KeyUp:
			case KeyDown:
				break;
			default: return;
		}
		m_move_keys[key] = false;
	}




    OrthoCamera::OrthoCamera(float left, float right, float bottom, float top, float near, float far)
    {
        create(left, right, bottom, top, near, far);
    }

	OrthoCamera& OrthoCamera::createDefault(float windowWidth, float WindowHeight, bool soft)
	{
		return create(0.0f, windowWidth * 1.0f, WindowHeight * 1.0f, 0.0f, -1.0f, 1.0f, soft);
	}

    OrthoCamera& OrthoCamera::create(float left, float right, float bottom, float top, float near, float far, bool soft)
    {
        m_projection = glm::ortho(left, right, bottom, top, near, far);
        __update_view_proj_mat();

		enableSignals();
		if (!soft)
			connectSignal(tBuiltinSignals::WindowResized);

        setTypeName("ox::OrthoCamera");
        validate();
        return *this;
    }

	void OrthoCamera::handleSignal(ostd::tSignal& signal)
	{
		if (signal.ID == tBuiltinSignals::WindowResized)
		{
			WindowSizeObj wsobj = static_cast<WindowSizeObj&>(signal.userData);
			createDefault(wsobj.width, wsobj.height, true);
		}
	}

    String OrthoCamera::toString(void) const
    {
        StringEditor se = getTypeName();
        se.add(" position = { ");
        se.addf(m_position.x).add(", ");
        se.addf(m_position.y).add(", ");
        se.addf(m_position.z).add(" } rotation = { ").addf(m_rotation).add(" } ");
        return se.str();
    }

    void OrthoCamera::__update_view_proj_mat(void)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position);
        transform = glm::rotate(transform, glm::radians(m_rotation), glm::vec3(0, 0, 1));

        m_view = glm::inverse(transform);
        m_viewProj = m_projection * m_view;
    }
}
