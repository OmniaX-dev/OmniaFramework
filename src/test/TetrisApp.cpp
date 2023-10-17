#include "TetrisApp.hpp"
#include <ostd/Signals.hpp>
#include <ogfx/ResourceManager.hpp>
#include <ogfx/Renderer2D.hpp>
#include <ostd/Random.hpp>

using namespace ostd;
using namespace ogfx;

TetrisPiece::TetrisPiece(ostd::Vec2 spawnPos)
{
	newRandomPiece();
	m_topLeft = spawnPos;
}

void TetrisPiece::rotate(void)
{
	m_currentPosition = (++m_currentPosition % 4);
}

void TetrisPiece::newRandomPiece(void)
{
	uint8_t pieceIndex = Random::geti8(0, 6);
	uint8_t rotationIndex = Random::geti8(0, 3);

	switch(pieceIndex)
	{
		case 0: copy_positions(TetrisPieces::O); break;
		case 1: copy_positions(TetrisPieces::I); break;
		case 2: copy_positions(TetrisPieces::S); break;
		case 3: copy_positions(TetrisPieces::Z); break;
		case 4: copy_positions(TetrisPieces::L); break;
		case 5: copy_positions(TetrisPieces::J); break;
		case 6: copy_positions(TetrisPieces::T); break;
	}

	m_currentPosition = rotationIndex;
}

void TetrisPiece::draw(ogfx::RenderCore& gfx, float tileSize, ostd::TextureID texture)
{
	for (uint8_t x = 0; x < 4; x++)
	{
		for (uint8_t y = 0; y < 4; y++)
		{
			if (m_positions[m_currentPosition][y][x] == 1)
			{
				ostd::Vec2 pos = m_topLeft;
				pos.x += (tileSize * x);
				pos.y += (tileSize * y);
				gfx.drawImage(texture, pos, { tileSize, tileSize }, { 128, 0, 0 }, false);
			}
			else
			{
				ostd::Vec2 pos = m_topLeft;
				pos.x += (tileSize * x);
				pos.y += (tileSize * y);
				gfx.drawImage(texture, pos, { tileSize, tileSize }, { 0, 128, 0, 50 }, false);
			}
		}
	}
}

void TetrisPiece::update(float tileSize, int32_t currentSpeed)
{
	if (m_currentStep++ < currentSpeed) return;
	m_currentStep = 0;
	m_topLeft.y += tileSize;
}

float TetrisPiece::getBottomPoint(float tileSize)
{
	for (int8_t y = 3; y >= 0; y--)
	{
		for (int8_t x = 0; x < 4; x++)
		{
			if (m_positions[m_currentPosition][y][x] == 1)
				return (y + 1) * tileSize;
		}
	}
	return 0;
}

float TetrisPiece::getTopPoint(float tileSize)
{
	for (int8_t y = 0; y < 4; y++)
	{
		for (int8_t x = 0; x < 4; x++)
		{
			if (m_positions[m_currentPosition][y][x] == 1)
				return y * tileSize;
		}
	}
	return 0;
}

float TetrisPiece::getLeftPoint(float tileSize)
{
	for (int8_t x = 0; x < 4; x++)
	{
		for (int8_t y = 0; y < 4; y++)
		{
			if (m_positions[m_currentPosition][y][x] == 1)
				return x * tileSize;
		}
	}
	return 0;
}

float TetrisPiece::getRightPoint(float tileSize)
{
	for (int8_t x = 3; x >= 0; x--)
	{
		for (int8_t y = 0; y < 4; y++)
		{
			if (m_positions[m_currentPosition][y][x] == 1)
				return (x + 1) * tileSize;
		}
	}
	return 0;
}

void TetrisPiece::moveLeft(float tileSize)
{
	m_topLeft.x -= tileSize;
}

void TetrisPiece::moveRight(float tileSize)
{
	m_topLeft.x += tileSize;
}

void TetrisPiece::copy_positions(const uint8_t pos[4][4][4])
{
	for (uint8_t i = 0; i < 4; i++)
	{
		for (uint8_t j = 0; j < 4; j++)
		{
			for (uint8_t k = 0; k < 4; k++)
			{
				m_positions[i][j][k] = pos[i][j][k];
			}
		}
	}
}



void TetrisApp::onSetup(void)
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

	tetrisTile.texture = ogfx::ResourceManager::loadTexture("textures/tetris.png");
	auto& tex = ogfx::ResourceManager::getTexture(tetrisTile.texture);
	tetrisTile.tile = tex.addTileInfo(0, 0, 32, 32);

	ogfx::DebugTools::m_debugWindowTabPanel.selectTab(1);

	topLeft.x = (getWindowWidth() / 2.0f) - ((tileSize * (hTiles + 2)) / 2.0f);
	topLeft.y = getWindowHeight() - (tileSize * (vTiles + 2));

	ostd::Vec2 piecePos = topLeft;
	piecePos.y += tileSize;
	piecePos.y -= piece.getBottomPoint(tileSize);
	piecePos.x += (((hTiles) * tileSize) - (2 * tileSize)) / 2.0f;
	piece.setPosition(piecePos);

	validate();
}

void TetrisApp::onUpdate(void)
{
	piece.update(tileSize, speed);
	blender.process(gfx);
}

void TetrisApp::onRender(void)
{
	gfx.bindShader(baseShader);	

	gfx.setRenderTarget(target);
	gfx.clear({ 10, 10, 10 });

	gfx.drawQuad(topLeft, { ((hTiles + 2) * tileSize), ((vTiles + 2) * tileSize) }, { 30, 30, 30 }, false);

	piece.draw(gfx, tileSize, tetrisTile);

	for (uint32_t x = 0; x < (hTiles + 2); x++)
	{
		gfx.drawImage(tetrisTile, { topLeft.x + (x * tileSize), topLeft.y - tileSize }, { tileSize, tileSize }, { 20, 20, 20 }, false);
		gfx.drawImage(tetrisTile, { topLeft.x + (x * tileSize), topLeft.y }, { tileSize, tileSize }, borderColor, false);
		gfx.drawImage(tetrisTile, { topLeft.x + (x * tileSize), getWindowHeight() - tileSize }, { tileSize, tileSize }, borderColor, false);
	}
	for (uint32_t y = 1; y <= vTiles; y++)
	{
		gfx.drawImage(tetrisTile, { topLeft.x, topLeft.y + (y * tileSize) }, { tileSize, tileSize }, borderColor, false);
		gfx.drawImage(tetrisTile, { topLeft.x + ((hTiles + 1) * tileSize), topLeft.y + (y * tileSize) }, { tileSize, tileSize }, borderColor, false);
	}

	gfx.setRenderTarget(guiLayer);
	gfx.clear({ 0, 0, 0, 0 });
	if (showDebugWindow)
		oxgui::WidgetManager::render(gfx);
	drawDebugInfo(gfx);

	blender.renderToScreen(gfx, baseShader);
}

void TetrisApp::onSignal(tSignal& signal)
{
}

void TetrisApp::onKeyPressed(Event& evt)
{
	if (evt.sf().key.code == sf::Keyboard::F12)
	{
		showDebugWindow = !showDebugWindow;
		ogfx::DebugTools::m_debugWindowTabPanel.selectTab((showDebugWindow ? 0 : 1));
	}
	else if (evt.sf().key.code == sf::Keyboard::W)
	{
		piece.rotate();
	}
	else if (evt.sf().key.code == sf::Keyboard::A)
	{
		if (piece.getPosition().x + piece.getLeftPoint(tileSize) > (topLeft.x + tileSize))
			piece.moveLeft(tileSize);
	}
	else if (evt.sf().key.code == sf::Keyboard::D)
	{
		if (piece.getPosition().x + piece.getRightPoint(tileSize) < (topLeft.x + tileSize + (hTiles * tileSize)))
			piece.moveRight(tileSize);
	}
}

void TetrisApp::onKeyReleased(Event& evt)
{

}

void TetrisApp::onMousePressed(Event& evt)
{

}

void TetrisApp::onMouseReleased(Event& evt)
{

}

void TetrisApp::onMouseMoved(Event& evt)
{

}
