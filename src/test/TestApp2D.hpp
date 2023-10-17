#pragma once

#include <ogfx/RenderCore.hpp>
#include <ogfx/BlendRenderer.hpp>
#include <ogfx/GraphicsApplication.hpp>
#include <ogfx/Widgets.hpp>

class TestWidget : public ogfx::oxgui::Widget
{
	public:
		void create(void);
		void renderContent(ogfx::RenderCore& gfx) override;
};

class TestApp2D : public ogfx::GraphicsApplication2D
{
	public:
		void onSetup(void) override;
		void onRender(void) override;
		void onUpdate(void) override;

		void onSignal(ostd::tSignal& signal) override;
		void onKeyPressed(ogfx::Event& evt) override;
		void onKeyReleased(ogfx::Event& evt) override;
		void onMousePressed(ogfx::Event& evt) override;
		void onMouseReleased(ogfx::Event& evt) override;
		void onMouseMoved(ogfx::Event& evt) override;

	private:
		ostd::ResourceID baseShader;
		ogfx::RenderCore gfx;
		ogfx::OrthoCamera camera;
		ogfx::RenderTarget target;
		ogfx::RenderTarget guiLayer;
		ogfx::BlendRenderer blender;

		ogfx::oxgui::TabPanel tabPanel;
		ogfx::oxgui::SubWindow subWin1;
		ogfx::oxgui::TabWidget tab1;
		ogfx::oxgui::TabWidget tab2;
		ogfx::oxgui::TabWidget tab3;
		TestWidget testWidget;

		// ogfx::oxgui::SubWindow panel2;
		// ogfx::oxgui::SubWindow panel3;
		// ogfx::oxgui::SubWindow subPanel1;
};

// #pragma once

// #include <BaseObject.hpp>
// #include <ogfx/Window.hpp>
// #include <ogfx/RenderCore.hpp>
// #include <ogfx/BlendRenderer.hpp>

// class TestApp2D : public ostd::BaseObject
// {
// 	public:
// 		TestApp2D(void);
// 		void handleSignal(ostd::tSignal& signal);
// 		void mainLoop(void);

// 	private:
// 		ogfx::Window window;
// 		ostd::ResourceID baseShader;
// 		ogfx::RenderCore gfx;
// 		ogfx::OrthoCamera camera;
// 		ogfx::RenderTarget target;
// 		ogfx::BlendRenderer blender;
// };