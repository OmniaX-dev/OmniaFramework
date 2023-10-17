#ifndef __BLEND_RENDERER_HPP__
#define __BLEND_RENDERER_HPP__

#include <ogfx/Camera.hpp>
#include <ogfx/RenderTarget.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove from header

	struct tBlendModes
	{
		inline static constexpr uint8_t Override = 0;
		inline static constexpr uint8_t Transparent = 1;
		inline static constexpr uint8_t Multiply = 2;
		inline static constexpr uint8_t Add = 3;
		inline static constexpr uint8_t Subtract = 4;
		inline static constexpr uint8_t Normal = 5;

		inline static constexpr uint8_t Invalid = 0xFF;
	};

	class RenderCore;
	class BlendRenderer : public BaseObject
	{
		public: struct tLayer
		{
			RenderTarget* target { nullptr };
			uint8_t blendMode { tBlendModes::Override };

			inline tLayer(void) {  }
			inline tLayer(RenderTarget& _target, uint8_t _blendMode) { target = &_target; blendMode = _blendMode; }
			inline bool isValid(void) { return m_valid && target != nullptr && blendMode != tBlendModes::Invalid && target->isValid(); }

			private:
				inline tLayer(bool v = false) { m_valid = v; }
				bool m_valid { true };

			friend class BlendRenderer;
		};

		public:
			inline BlendRenderer(void) { invalidate(); }
			inline BlendRenderer(IPoint screenSize, OrthoCamera& camera) { create(screenSize, camera); }
			BlendRenderer& create(IPoint screenSize, OrthoCamera& camera);
			LayerID addLayer(RenderTarget& target, uint8_t blendMode = tBlendModes::Override);
			void process(RenderCore& gfx);
			void renderTo(RenderCore& gfx, const RenderTarget& target, ResourceID shader);
			void renderToScreen(RenderCore& gfx, ResourceID shader);
			bool overrideDefaultBlendShader(ResourceID shader);
			void setDefaultBlendShader(void);
			tLayer& getLayer(LayerID layer);
			void handleSignal(tSignal& signal) override;
			inline RenderTarget& getDestination(void) { return m_destination; }

		private:
			RenderTarget m_destination;
			ResourceID m_shader;
			std::vector<tLayer> m_layers;
			IPoint m_screenSize;
			std::vector<int32_t> m_blendModes;
			OrthoCamera* m_camera { nullptr };
			inline static LayerID s_nextID = 0;
			inline static tLayer s_invalidLayer { false };

		public:
			inline static constexpr int32_t MaxLayers = 14;
			inline static constexpr LayerID InvalidLayer = 1024;
	};
}

#endif