// #include "TestApp2D.hpp"
#include "TestApp3D.hpp"

int main(int argc, char** argv)
{
	// ogfx::DebugTools::enableDebugWindow();
	// ogfx::DebugTools::m_drawBasicFrameInfo = true;

	// TestApp3D_2 app;
	// app.create(32*40, 720, "OmniaFramework TEST");
	// while (app.isRunning())
	// 	app.nextFrame();
	// app.destroy();

	//TODO: I changed the endianness of ostd::serial::SerialIO and now the app crashes
	//		probably due to SerialIO beeing used in OX3DLoader
	TestApp3D app;
	app.gameLoop();

	return 0;
}
