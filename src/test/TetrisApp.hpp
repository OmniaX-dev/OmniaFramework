#pragma once

#include <ogfx/RenderCore.hpp>
#include <ogfx/BlendRenderer.hpp>
#include <ogfx/GraphicsApplication.hpp>
#include <ogfx/Widgets.hpp>

struct TetrisPieces
{
	inline static constexpr uint8_t O[4][4][4] = {
		{	{ 0, 1, 1, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 1, 1, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 1, 1, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 1, 1, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 } }
	};
	inline static constexpr uint8_t I[4][4][4] = {
		{	{ 0, 0, 0, 0 },
			{ 1, 1, 1, 1 },
			{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 0, 1, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 1, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			{ 1, 1, 1, 1 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 } }
	};
	inline static constexpr uint8_t S[4][4][4] = {
		{	{ 0, 0, 0, 0 },
			{ 0, 1, 1, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 0, 1, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			{ 0, 1, 1, 0 },
			{ 1, 1, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 1, 0, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 1, 0, 0 } }
	};
	inline static constexpr uint8_t Z[4][4][4] = {
		{	{ 0, 0, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 1, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 1, 1, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 1, 0, 0, 0 } }
	};
	inline static constexpr uint8_t L[4][4][4] = {
		{	{ 0, 0, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 1, 1, 1, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 1, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			{ 1, 1, 1, 0 },
			{ 1, 0, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 } }
	};
	inline static constexpr uint8_t J[4][4][4] = {
		{	{ 0, 0, 0, 0 },
			{ 1, 0, 0, 0 },
			{ 1, 1, 1, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			{ 1, 1, 1, 0 },
			{ 0, 0, 1, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 1, 1, 0, 0 } }
	};
	inline static constexpr uint8_t T[4][4][4] = {
		{	{ 0, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 1, 1, 1, 0 },
			{ 0, 0, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 1, 0 },
			{ 0, 1, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			{ 1, 1, 1, 0 },
			{ 0, 1, 0, 0 } },

		{	{ 0, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 1, 0, 0 } }
	};
};

class TetrisPiece
{
	public:
		TetrisPiece(ostd::Vec2 spawnPos = { 0.0f, 0.0f });
		void rotate(void);
		void newRandomPiece(void);
		void draw(ogfx::RenderCore& gfx, float tileSize, ostd::TextureID texture);
		void update(float tileSize, int32_t currentSpeed);
		float getBottomPoint(float tileSize);
		float getTopPoint(float tileSize);
		float getLeftPoint(float tileSize);
		float getRightPoint(float tileSize);
		void moveLeft(float tileSize);
		void moveRight(float tileSize);
		
		inline void setPosition(ostd::Vec2 pos) { m_topLeft = pos; }
		inline ostd::Vec2 getPosition(void) { return m_topLeft; }

	private:
		void copy_positions(const uint8_t pos[4][4][4]);

	private:
		uint8_t m_positions[4][4][4];
		uint8_t m_currentPosition { 0 };

		ostd::Vec2 m_topLeft { 0.0f, 0.0f };
		int32_t m_currentStep { 0 };

};

class TetrisApp : public ogfx::GraphicsApplication2D
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
		ostd::TextureID tetrisTile;

		bool showDebugWindow { false };

		ostd::Color borderColor { 50, 50, 50 };
		float tileSize = 32.0f;
		float hTiles = 10.0f;
		float vTiles = 20.0f;
		ostd::Vec2 topLeft { 0.0f, 0.0f };

		TetrisPiece piece;
		uint32_t speed { 60 };
};