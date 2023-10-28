// #include "TestApp2D.hpp"
#include "TestApp3D.hpp"
// #include "TetrisApp.hpp"

int main(int argc, char** argv)
{
	// ogfx::DebugTools::enableDebugWindow();
	// ogfx::DebugTools::m_drawBasicFrameInfo = true;

	// TetrisApp app;
	// app.create(32*40, 720, "OmniaFramework TEST");
	// while (app.isRunning())
	// 	app.nextFrame();
	// app.destroy();

	TestApp3D app;
	app.gameLoop();

	return 0;
}
