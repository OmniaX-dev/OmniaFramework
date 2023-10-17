#ifndef __RESOURCE_MANAGER_HPP__
#define __RESOURCE_MANAGER_HPP__

#include <ostd/Types.hpp>
#include <unordered_map>
#include <ogfx/Shader.hpp>
//#include <Audio.hpp>
#include <ogfx/Texture.hpp>
#include <ogfx/BitmapFont.hpp>

namespace ogfx
{
	using namespace ostd; //TODO: Remove from header

	struct tResourceType
	{
		inline static constexpr uint8_t Invalid = 0;
		inline static constexpr uint8_t Shader = 1;
		inline static constexpr uint8_t Texture = 2;
		inline static constexpr uint8_t BitmapFont = 3;
		inline static constexpr uint8_t SoundBuffer = 4;
	};

	class ResourceManager
	{
		public:
			static void init(void);
			static ResourceID loadShader(const String& vertex_file_path, const String& fragment_file_path);
			static ResourceID loadShader(const String& shader_name, bool single_file = true, const String& shader_folder = "");
			static ResourceID newShader(const String& vertex_shader_source, const String& fragment_shader_source);
			static ResourceID loadTexture(const String& filePath,
										bool store_data = false,
										int32_t min_filter_mode = GL_LINEAR,
										int32_t mag_filter_mode = GL_LINEAR,
										int32_t wrap_s_mode = GL_CLAMP_TO_EDGE,
										int32_t wrap_t_mode = GL_CLAMP_TO_EDGE);
			static ResourceID loadTexture(const unsigned char* data,
										  unsigned int data_size,
										  bool store_data = false,
										  int32_t min_filter_mode = GL_LINEAR,
										  int32_t mag_filter_mode = GL_LINEAR,
										  int32_t wrap_s_mode = GL_CLAMP_TO_EDGE,
										  int32_t wrap_t_mode = GL_CLAMP_TO_EDGE);
			static ResourceID loadTexture(const unsigned char* rawData,
										  unsigned int data_size,
										  int32_t width,
										  int32_t height,
										  int32_t bpp = 4,
										  int32_t min_filter_mode = GL_LINEAR,
										  int32_t mag_filter_mode = GL_LINEAR,
										  int32_t wrap_s_mode = GL_CLAMP_TO_EDGE,
										  int32_t wrap_t_mode = GL_CLAMP_TO_EDGE);
			static ResourceID newTexture(int32_t width, int32_t height);
			static ResourceID loadBitmapFont(const String& filePath, bool monospace = false);
			static ResourceID loadBitmapFont(const unsigned char* data, unsigned int data_size, bool monospace = false);
			//static ResourceID loadSound(const String& filePath);
			//static SoundInstance newSoundInstance(ResourceID soundBuffer, float volume = 100.0f, bool loop = false);

			static bool destroyResource(ResourceID resource, uint8_t res_type);

			static Shader& getShader(ResourceID id);
			static Texture& getTexture(ResourceID id);
			static BitmapFont& getBitmapFont(ResourceID id);
			//static SoundBuffer& getSoundBuffer(ResourceID id);

			inline static ResourceID getDefaultShader(void) { return ResourceManager::s_defaultShader; }
			inline static ResourceID getDefaultBlendShader(void) { return ResourceManager::s_defaultBlendShader; }
			inline static ResourceID getDefaultBitmapFont(void) { return ResourceManager::s_defaultBitmapFont; }
			inline static ResourceID getDefaultBitmapFontMono(void) { return ResourceManager::s_defaultBitmapFontMono; }
			inline static ResourceID getDefaultLightTexture(void) { return ResourceManager::s_defaultLightTexture; }
			inline static ResourceID getStateGradientTexture(void) { return ResourceManager::s_stateGradientTexture; }
			inline static ResourceID getBasicShadowTexture(void) { return ResourceManager::s_basicShadowTexture; }

		private:
			inline static std::unordered_map<ResourceID, Shader> s_shaders;
			inline static std::unordered_map<ResourceID, Texture> s_textures; 
			inline static std::unordered_map<ResourceID, BitmapFont> s_bitmapFonts; 
			//inline static std::unordered_map<ResourceID, SoundBuffer> s_soundBuffers;
			inline static ResourceID s_next_id = 1024;
			inline static ResourceID s_defaultShader { 0 };
			inline static ResourceID s_defaultBlendShader { 0 };
			inline static ResourceID s_defaultBitmapFont { 0 };
			inline static ResourceID s_defaultBitmapFontMono { 0 };
			inline static ResourceID s_defaultLightTexture { 0 };
			inline static ResourceID s_ghostTexture { 0 };
			inline static ResourceID s_ghostShader { 0 };
			inline static ResourceID s_ghostBitmapFont { 0 };
			inline static ResourceID s_stateGradientTexture { 0 };
			inline static ResourceID s_basicShadowTexture { 0 };
			inline static ResourceID s_ghostSoundBuffer { 0 };

		public:
			inline static const uint8_t InvalidResource = 0;
	};
} // namespace ox

#endif
