#include "Texture.hpp"
#ifndef STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image/stb_image.h>
#include <Logger.hpp>
#include "Errors.hpp"

//TODO: Implement Errors

namespace ogfx
{
	using namespace ostd;

	Texture::Texture(const String& path, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode)
	{
		create(path, store_data, min_filter_mode, mag_filter_mode, wrap_s_mode, wrap_t_mode);;
	}

	Texture::Texture(int32_t width, int32_t height)
	{
		create(width, height);
	}

	Texture::~Texture(void)
	{
		uint32_t id = getOpenGLID();
		GLCall(glDeleteTextures(1, &id));
		setID(id);
		invalidate();
	}

	Texture& Texture::create(const String& path, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode)
	{
		invalidate();
		stbi_set_flip_vertically_on_load(1);
		if (m_localData != nullptr)
			stbi_image_free(m_localData);
		m_localData = nullptr;
		m_filePath = "";
		m_bpp = 0;
		m_dataStored = false;
		setID(0);
		m_localData = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4);
		if (m_localData == nullptr)
		{
			ErrorHandler::pushError(Texture::ERR_IMAGE_LOAD_FAILED);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("%s\nFile Path: %s", err_str.c_str(), path.c_str());
			return *this;
		}
		uint32_t gl_id;
		GLCall(glGenTextures(1, &gl_id));
		setID(gl_id);
		bind();

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t_mode));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localData));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
		//unbind();

		m_dataStored = store_data;
		if (!m_dataStored)
		{
			stbi_image_free(m_localData);
			m_localData = nullptr;
		}

		m_filePath = path;
		setTypeName("ox::Texture");
		validate();
		return *this;
	}

	Texture& Texture::create(int32_t width, int32_t height)
	{
		invalidate();
		if (m_localData != nullptr)
			stbi_image_free(m_localData);
		m_localData = nullptr;
		m_filePath = "";
		m_bpp = 0;
		m_dataStored = false;
		setID(0);
		m_width = width;
		m_height = height;
		uint32_t gl_id;
		GLCall(glGenTextures(1, &gl_id));
		setID(gl_id);
		bind();

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localData));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));

		setTypeName("ox::Texture");
		validate();
		return *this;
	}
	
	Texture& Texture::create(const unsigned char* data, unsigned int data_size, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode)
	{
		invalidate();
		stbi_set_flip_vertically_on_load(1);
		if (m_localData != nullptr)
			stbi_image_free(m_localData);
		m_localData = nullptr;
		m_filePath = "";
		m_bpp = 0;
		m_dataStored = false;
		setID(0);
		m_localData = stbi_load_from_memory(data, data_size, &m_width, &m_height, &m_bpp, 4);
		if (m_localData == nullptr)
		{
			ErrorHandler::pushError(Texture::ERR_IMAGE_LOAD_FAILED);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("%s\nMemory://data", err_str.c_str());
			return *this;
		}
		uint32_t gl_id;
		GLCall(glGenTextures(1, &gl_id));
		setID(gl_id);
		bind();

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t_mode));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localData));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
		//unbind();

		m_dataStored = store_data;
		if (!m_dataStored)
		{
			stbi_image_free(m_localData);
			m_localData = nullptr;
		}

		setTypeName("ox::Texture");
		validate();
		return *this;
	}

	Texture& Texture::createFromRawData(const unsigned char* rawData, unsigned int data_size, int32_t width, int32_t height, int32_t bpp, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode)
	{
		invalidate();
		stbi_set_flip_vertically_on_load(1);
		if (m_localData != nullptr && m_dataStored)
			stbi_image_free(m_localData);
		else if (m_localData != nullptr && m_dataCopied)
			delete[] m_localData;
		m_localData = new uint8_t[data_size];
		for (uint32_t i = 0; i < data_size; i++)
			m_localData[i] = rawData[i];
		m_filePath = "";
		m_bpp = bpp;
		m_width = width;
		m_height = height;
		m_dataStored = false;
		m_dataCopied = true;

		uint32_t gl_id;
		GLCall(glGenTextures(1, &gl_id));
		setID(gl_id);
		bind();

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s_mode));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t_mode));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localData));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));

		setTypeName("ox::Texture");
		validate();
		return *this;
	}


	const uint8_t* Texture::getPixelData(void)
	{
		if (!hasDataStored())
		{
			ErrorHandler::pushError(Texture::ERR_NO_DATA_STORED);
			String err_str = ErrorHandler::getLastErrorString();
			OX_ERROR("%s", err_str.c_str());
			return nullptr;
		}
		return m_localData;
	}
	
	void Texture::bind(uint32_t slot) const
	{
		GLCall(glActiveTexture(GL_TEXTURE0 + slot));
		GLCall(glBindTexture(GL_TEXTURE_2D, getOpenGLID()));
	}
	
	void Texture::unbind(void) const
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	}

	TextureAtlasIndex Texture::addTileInfo(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
	{
		if (isInvalid() || x + w > m_width || y + h > m_height)
		{
			OX_WARN("ox::Texture::addTileInfo(...): Invalid texture coordinates.");
			return Texture::FullTextureCoords;
		}
		if (!hasTileData())
			m_tiles.push_back(tTexCoords());
		Vec2 bottomLeft = { (float)x / (float)m_width, 1.0f - ((float)(y + h) / (float)m_height) };
		Vec2 bottomRight = { (float)(x + w) / (float)m_width, 1.0f - ((float)(y + h) / (float)m_height) };
		Vec2 topLeft = { (float)x / (float)m_width, 1.0f - ((float)y / (float)m_height) };
		Vec2 topRight = { (float)(x + w) / (float)m_width, 1.0f - ((float)y / (float)m_height) };
		tTexCoords texc;
		texc.bottomLeft = bottomLeft;
		texc.bottomRight = bottomRight;
		texc.topLeft = topLeft;
		texc.topRight = topRight;
		m_tiles.push_back(texc);
		return m_tiles.size() - 1;
	}

	Texture::tTexCoords Texture::getTile(TextureAtlasIndex index)
	{
		if (!hasTileData() || index >= m_tiles.size()) 
		{
			//OX_WARN("ox::Texture::getTile(...): Unable to retrieve tile.");
			return Texture::tTexCoords();
		}
		return m_tiles[index];
	}




	CubeTexture::CubeTexture(const std::vector<String>& paths, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode, int32_t wrap_r_mode)
	{
		create(paths, store_data, min_filter_mode, mag_filter_mode, wrap_s_mode, wrap_t_mode, wrap_r_mode);
	}

	CubeTexture::~CubeTexture(void)
	{
		uint32_t id = getOpenGLID();
		GLCall(glDeleteTextures(1, &id));
		setID(id);
		invalidate();
	}

	CubeTexture& CubeTexture::create(const std::vector<String>& paths, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode, int32_t wrap_r_mode)
	{
		invalidate();
		if (paths.size() != 6) return *this; //TODO: Error
		stbi_set_flip_vertically_on_load(0);
		for (auto& tex_data : m_localData)
		{
			if (tex_data != nullptr)
				stbi_image_free(tex_data);
		}
		m_localData.clear();
		m_bpp = 0;
		m_dataStored = false;
		setID(0);
		for (uint32_t i = 0; i < 6; i++)
		{
			m_localData.push_back(stbi_load(paths[i].c_str(), &m_width, &m_height, &m_bpp, 4));
			//TODO: Check for different sizes and error if not equal
			if (m_localData[i] == nullptr)
			{
				ErrorHandler::pushError(Texture::ERR_IMAGE_LOAD_FAILED);
				String err_str = ErrorHandler::getLastErrorString();
				OX_ERROR("%s\nFile Path: %s", err_str.c_str(), paths[i].c_str());
				return *this;
			}
		}
		
		uint32_t gl_id;
		GLCall(glGenTextures(1, &gl_id));
		setID(gl_id);
		bind();

		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r_mode));

		for (uint32_t i = 0; i < 6; i++)
		{
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localData[i]));
		}

		m_dataStored = store_data;
		if (!m_dataStored)
		{
			for (auto& tex_data : m_localData)
			{
				stbi_image_free(tex_data);
			}
			m_localData.clear();
		}

		setTypeName("ox::CubeTexture");
		validate();
		return *this;
	}
	
	CubeTexture& CubeTexture::create(const std::vector<const unsigned char*>& data, unsigned int data_size, bool store_data, int32_t min_filter_mode, int32_t mag_filter_mode, int32_t wrap_s_mode, int32_t wrap_t_mode, int32_t wrap_r_mode)
	{
		invalidate();
		if (data.size() != 6) return *this; //TODO: Error
		stbi_set_flip_vertically_on_load(1);
		for (auto& tex_data : m_localData)
		{
			if (tex_data != nullptr)
				stbi_image_free(tex_data);
		}
		m_localData.clear();
		m_bpp = 0;
		m_dataStored = false;
		setID(0);
		for (uint32_t i = 0; i < 6; i++)
		{
			m_localData.push_back(stbi_load_from_memory(data[i], data_size, &m_width, &m_height, &m_bpp, 4));
			//TODO: Check for different sizes and error if not equal
			if (m_localData[i] == nullptr)
			{
				ErrorHandler::pushError(Texture::ERR_IMAGE_LOAD_FAILED);
				String err_str = ErrorHandler::getLastErrorString();
				OX_ERROR("%s\nMemory://data", err_str.c_str());
				return *this;
			}
		}
		
		uint32_t gl_id;
		GLCall(glGenTextures(1, &gl_id));
		setID(gl_id);
		bind();

		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t_mode));
		GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r_mode));

		for (uint32_t i = 0; i < 6; i++)
		{
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localData[i]));
		}

		m_dataStored = store_data;
		if (!m_dataStored)
		{
			for (auto& tex_data : m_localData)
			{
				stbi_image_free(tex_data);
			}
			m_localData.clear();
		}

		setTypeName("ox::CubeTexture");
		validate();
		return *this;
	}
	
	void CubeTexture::bind(uint32_t slot) const
	{
		GLCall(glActiveTexture(GL_TEXTURE0 + slot));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, getOpenGLID()));
	}
	
	void CubeTexture::unbind(void) const
	{
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}
}
