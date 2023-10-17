#include "TestApp2D.hpp"
#include <ostd/Signals.hpp>
#include <ogfx/ResourceManager.hpp>
#include <ogfx/Renderer2D.hpp>
#include <ostd/Random.hpp>

using namespace ostd;
using namespace ogfx;


void TestWidget::create(void)
{
	setSize(120, 50);
	setPosition(100, 100);
	validate();
	enable();
	connectSignals();
}

void TestWidget::renderContent(ogfx::RenderCore& gfx)
{
	// std::cout << getPosition() << "\n";
	gfx.drawQuad(getPosition(), getSize(), { 255, 0, 128 }, false);
}


void TestApp2D::onSetup(void)
{
	baseShader = ResourceManager::getDefaultShader();
	gfx.init();

	camera.createDefault(getWindowWidth(), getWindowHeight());
	gfx.setCamera(camera);

	target.create(getWindowWidth(), getWindowHeight());
	guiLayer.create(getWindowWidth(), getWindowHeight());

	blender.create({ getWindowWidth(), getWindowHeight() }, camera);
	blender.addLayer(target, tBlendModes::Override);
	blender.addLayer(guiLayer, tBlendModes::Normal);

	subWin1.create({ 100, 100 }, { 500, 500 });
	subWin1.enable();

	oxgui::WidgetManager::addWidget(subWin1);

	tabPanel.create(subWin1);
	tabPanel.addTab(tab1);
	tabPanel.addTab(tab2);
	tabPanel.addTab(tab3);
	tab1.setText("Tab1");
	tab2.setText("Tab2");
	tab3.setText("Tab3");

	testWidget.create();
	tab2.addWidget(testWidget);

	validate();
}

void TestApp2D::onUpdate(void)
{
	blender.process(gfx);
}

void TestApp2D::onRender(void)
{
	gfx.bindShader(baseShader);	
	gfx.setRenderTarget(target);
	gfx.clear({ 20, 50, 70 });
	gfx.drawQuad({ 100, 100 }, { 50, 100 }, { 200, 10, 10 });
	gfx.drawText("THIS IS TEXT", Vec2 { 100, 100 }, (float) 40, Color { 255, 255, 0 });

	gfx.setRenderTarget(guiLayer);
	gfx.clear({ 0, 0, 0, 0 });
	oxgui::WidgetManager::render(gfx);
	drawDebugInfo(gfx);

	// drawDebugInfo(gfx);

	blender.renderToScreen(gfx, baseShader);
}

void TestApp2D::onSignal(tSignal& signal)
{
}

void TestApp2D::onKeyPressed(Event& evt)
{
}

void TestApp2D::onKeyReleased(Event& evt)
{

}

void TestApp2D::onMousePressed(Event& evt)
{

}

void TestApp2D::onMouseReleased(Event& evt)
{

}

void TestApp2D::onMouseMoved(Event& evt)
{

}
