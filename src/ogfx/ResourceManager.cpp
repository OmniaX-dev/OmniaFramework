#include "ResourceManager.hpp"
#include <ogfx/static_resources/default_shaders.hpp>
#include <ogfx/static_resources/default_bitmap_font.hpp>
#include <ogfx/static_resources/default_light_texture.hpp>
#include <ogfx/static_resources/StateGradient.hpp>
#include <ogfx/static_resources/BasicShadowTexture.hpp>
#include <Logger.hpp>

namespace ogfx
{
	using namespace ostd;

	void ResourceManager::init(void)
	{
		ResourceManager::s_defaultShader = ResourceManager::newShader(oxres::defaultShader_vert, oxres::defaultShader_frag);
		ResourceManager::s_defaultBlendShader = ResourceManager::newShader(oxres::defaultBlendShader_vert, oxres::defaultBlendShader_frag);
		ResourceManager::s_defaultBitmapFont = ResourceManager::loadBitmapFont(oxres::default_bitmap_font.data, oxres::default_bitmap_font.size);
		ResourceManager::s_defaultBitmapFontMono = ResourceManager::loadBitmapFont(oxres::default_bitmap_font.data, oxres::default_bitmap_font.size, true);
		int32_t samplers[16];
		for (uint8_t i = 0; i < 16; i++)
			samplers[i] = i;
		auto& defShad = ResourceManager::getShader(ResourceManager::s_defaultShader);
		defShad.bind();
		defShad.updateUniform_arri("u_textures", 16, samplers);
		auto& blendShad = ResourceManager::getShader(ResourceManager::s_defaultBlendShader);
		blendShad.bind();
		blendShad.updateUniform_arri("u_textures", 16, samplers);
		blendShad.unbind();
		ResourceManager::s_defaultLightTexture = ResourceManager::loadTexture(oxres::default_light_texture.data, oxres::default_light_texture.size);
		ResourceManager::s_stateGradientTexture = ResourceManager::loadTexture(oxres::StateGradient.data, oxres::StateGradient.size, true);
		ResourceManager::s_basicShadowTexture = ResourceManager::loadTexture(oxres::BasicShadowTexture.data, oxres::BasicShadowTexture.size, true);
	}

	ResourceID ResourceManager::loadShader(const String& vertex_file_path, const String& fragment_file_path)
	{
		auto id = (ResourceManager::s_ghostShader != 0 ? ResourceManager::s_ghostShader : ResourceManager::s_next_id++);
		ResourceManager::s_shaders[id].createFromSeparateFiles(vertex_file_path, fragment_file_path);
		if (ResourceManager::s_shaders[id].isInvalid())
		{
			ResourceManager::s_ghostShader = id;
			OX_WARN("ox::ResourceManager::loadShader(const ox::String&, const ox::String&): Failed to load shader:\nVertex Shader path: %s\nFragment Shader path: %s", vertex_file_path.c_str(), fragment_file_path.c_str());
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostShader = 0;
		return id;
	}

	ResourceID ResourceManager::loadShader(const String& shader_name, bool single_file, const String& shader_folder)
	{
		auto id = (ResourceManager::s_ghostShader != 0 ? ResourceManager::s_ghostShader : ResourceManager::s_next_id++);
		ResourceManager::s_shaders[id].createFromName(shader_name, single_file, shader_folder);
		if (ResourceManager::s_shaders[id].isInvalid())
		{
			ResourceManager::s_ghostShader = id;
			OX_WARN("ox::ResourceManager::loadShader(const ox::String&, bool, const String&): Failed to load shader:\nShader name: %s\nShader folder: %s", shader_name.c_str(), shader_folder.c_str());
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostShader = 0;
		return id;
	}

	ResourceID ResourceManager::newShader(const String& vertex_shader_source, const String& fragment_shader_source)
	{
		auto id = (ResourceManager::s_ghostShader != 0 ? ResourceManager::s_ghostShader : ResourceManager::s_next_id++);
		ResourceManager::s_shaders[id].createFromSources(vertex_shader_source, fragment_shader_source);
		if (ResourceManager::s_shaders[id].isInvalid())
		{
			ResourceManager::s_ghostShader = id;
			OX_WARN("ox::ResourceManager::newShader(const ox::String&, const String&): Failed to create shader.");
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostShader = 0;
		return id;
	}

	ResourceID ResourceManager::loadTexture(const String& filePath, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode)
	{
		auto id = (ResourceManager::s_ghostTexture != 0 ? ResourceManager::s_ghostTexture : ResourceManager::s_next_id++);
		ResourceManager::s_textures[id].create(filePath, store_data, min_filter_mode, mag_filter_mode, wrap_s_mode, wrap_t_mode);
		if (ResourceManager::s_textures[id].isInvalid())
		{
			ResourceManager::s_ghostTexture = id;
			OX_WARN("ox::ResourceManager::loadTexture(const ox::String&, bool, int32_t, int32_t, int32_t, int32_t): Failed to load texture:\nTexture path: %s", filePath.c_str());
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostTexture = 0;
		return id;
	}

	ResourceID ResourceManager::loadTexture(const unsigned char* data, uint32_t data_size, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode)
	{
		auto id = (ResourceManager::s_ghostTexture != 0 ? ResourceManager::s_ghostTexture : ResourceManager::s_next_id++);
		ResourceManager::s_textures[id].create(data, data_size, store_data, min_filter_mode, mag_filter_mode, wrap_s_mode, wrap_t_mode);
		if (ResourceManager::s_textures[id].isInvalid())
		{
			ResourceManager::s_ghostTexture = id;
			OX_WARN("ox::ResourceManager::loadTexture(const unsigned char*, uint32_t, bool, int32_t, int32_t, int32_t, int32_t): Failed to load texture from memory.");
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostTexture = 0;
		return id;
	}

	ResourceID ResourceManager::loadTexture(const unsigned char* rawData, uint32_t data_size, int32_t width, int32_t height, int32_t bpp, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode)
	{
		auto id = (ResourceManager::s_ghostTexture != 0 ? ResourceManager::s_ghostTexture : ResourceManager::s_next_id++);
		ResourceManager::s_textures[id].createFromRawData(rawData, data_size, width, height, bpp, min_filter_mode, mag_filter_mode, wrap_s_mode, wrap_t_mode);
		if (ResourceManager::s_textures[id].isInvalid())
		{
			ResourceManager::s_ghostTexture = id;
			OX_WARN("ox::ResourceManager::loadTexture(const unsigned char*, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t): Failed to load texture from memory.");
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostTexture = 0;
		return id;
	}

	ResourceID ResourceManager::newTexture(int32_t width, int32_t height)
	{
		auto id = (ResourceManager::s_ghostTexture != 0 ? ResourceManager::s_ghostTexture : ResourceManager::s_next_id++);
		ResourceManager::s_textures[id].create(width, height);
		if (ResourceManager::s_textures[id].isInvalid())
		{
			ResourceManager::s_ghostTexture = id;
			OX_WARN("ox::ResourceManager::newTexture(int32_t, int32_t): Failed to create texture.");
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostTexture = 0;
		return id;
	}

	ResourceID ResourceManager::loadBitmapFont(const String& filePath, bool monospace)
	{
		auto id = (ResourceManager::s_ghostBitmapFont != 0 ? ResourceManager::s_ghostBitmapFont : ResourceManager::s_next_id++);
		ResourceManager::s_bitmapFonts[id].create(filePath, monospace);
		if (ResourceManager::s_bitmapFonts[id].isInvalid())
		{
			ResourceManager::s_ghostBitmapFont = id;
			OX_WARN("ox::ResourceManager::loadBitmapFont(const String&): Failed to load bitmap font:\nFile path: %s", filePath.c_str());
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostBitmapFont = 0;
		return id;
	}

	ResourceID ResourceManager::loadBitmapFont(const unsigned char* data, uint32_t data_size, bool monospace)
	{
		auto id = (ResourceManager::s_ghostBitmapFont != 0 ? ResourceManager::s_ghostBitmapFont : ResourceManager::s_next_id++);
		auto ftex = loadTexture(data, data_size, true, GL_NEAREST, GL_NEAREST);
		ResourceManager::s_bitmapFonts[id].create(ftex, monospace);
		if (ResourceManager::s_bitmapFonts[id].isInvalid())
		{
			ResourceManager::s_ghostBitmapFont = id;
			OX_WARN("ox::ResourceManager::loadBitmapFont(const unsigned char*, uint32_t): Failed to load bitmap font from memory.");
			return ResourceManager::InvalidResource;
		}
		ResourceManager::s_ghostBitmapFont = 0;
		return id;
	}

//	ResourceID ResourceManager::loadSound(const String& filePath)
//	{
//		auto id = (ResourceManager::s_ghostSoundBuffer != 0 ? ResourceManager::s_ghostSoundBuffer : ResourceManager::s_next_id++);
//		ResourceManager::s_soundBuffers[id].create(filePath);
//		if (ResourceManager::s_soundBuffers[id].isInvalid())
//		{
//			ResourceManager::s_ghostSoundBuffer = id;
//			OX_WARN("ox::ResourceManager::loadSoundBuffer(const String&): Failed to load sound buffer:\nFile path: %s", filePath.c_str());
//			return ResourceManager::InvalidResource;
//		}
//		ResourceManager::s_ghostSoundBuffer = 0;
//		return id;
//	}

//	SoundInstance ResourceManager::newSoundInstance(ResourceID soundBuffer, float volume, bool loop)
//	{
//		auto& sbuff = ResourceManager::getSoundBuffer(soundBuffer);
//		if (sbuff.isInvalid())
//		{
//			//TODO: Error
//			return *((SoundInstance*)&BaseObject::InvalidRef());
//		}
//		SoundInstance sound(sbuff);
//		if (sound.isInvalid())
//		{
//			//TODO: Error
//			return *((SoundInstance*)&BaseObject::InvalidRef());
//		}
//		return sound.setVolume(volume).enableLoop(loop);
//	}

	bool ResourceManager::destroyResource(ResourceID resource, uint8_t res_type)
	{
		if (res_type == tResourceType::Texture)
		{
			auto it = ResourceManager::s_textures.find(resource);
			if (it == ResourceManager::s_textures.end())
			{
				OX_WARN("ox::ResourceManager::destroyResource(...): Attempting to destroy an invalid texture.");
				return false;
			}
			ResourceManager::s_textures.erase(it);
			return true;
		}
		else if (res_type == tResourceType::Shader)
		{
			return false;
		}
		else if (res_type == tResourceType::BitmapFont)
		{
			return false;
		}
		else if (res_type == tResourceType::SoundBuffer)
		{
			return false;
		}
		else
		{
			OX_WARN("ox::ResourceManager::destroyResource(...): Invalid resource type.");
			return false;
		}
		return false;
	}

	Shader& ResourceManager::getShader(ResourceID id)
	{
		if (ResourceManager::s_shaders.find(id) == ResourceManager::s_shaders.end())
		{
			OX_WARN("ox::ResourceManager::getShader(...): Attempting to retrieve an invalid shader.");
			OX_DEBUG("SHADER_RES_ID: %d", id);
			return (Shader&)BaseObject::InvalidRef();
		}
		return ResourceManager::s_shaders[id];
	}
	
	Texture& ResourceManager::getTexture(ResourceID id)
	{
		if (ResourceManager::s_textures.find(id) == ResourceManager::s_textures.end())
		{
			OX_WARN("ox::ResourceManager::getTexture(...): Attempting to retrieve an invalid texture.");
			return (Texture&)BaseObject::InvalidRef();
		}
		return ResourceManager::s_textures[id];
	}

	BitmapFont& ResourceManager::getBitmapFont(ResourceID id)
	{
		if (ResourceManager::s_bitmapFonts.find(id) == ResourceManager::s_bitmapFonts.end())
		{
			OX_WARN("ox::ResourceManager::getBitmapFont(...): Attempting to retrieve an invalid bitmap font.");
			OX_DEBUG("DEFAULT_BITMAP_FONT: %d", s_defaultBitmapFont);
			OX_DEBUG("CURRENT_BITMAP_FONT: %d", id);
			OX_DEBUG("TOTAL_BITMAP_FONTS: %d", s_bitmapFonts.size());
			return (BitmapFont&)BaseObject::InvalidRef();
		}
		return ResourceManager::s_bitmapFonts[id];
	}

//	SoundBuffer& ResourceManager::getSoundBuffer(ResourceID id)
//	{
//		if (ResourceManager::s_soundBuffers.find(id) == ResourceManager::s_soundBuffers.end())
//		{
//			OX_WARN("ox::ResourceManager::getSoundBuffer(...): Attempting to retrieve an invalid sound buffer.");
//			return (SoundBuffer&)BaseObject::InvalidRef();
//		}
//		return ResourceManager::s_soundBuffers[id];
//	}
} // namespace ox
