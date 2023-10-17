#include "OX3DLoader.hpp"
#include "Serial.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "ogfx/DataStructures.hpp"
#include "ogfx/ResourceManager.hpp"
#include <cstdint>
#include <fstream>
#include <vector>

namespace ogfx
{
	using namespace ostd;

	Mesh OX3DLoader::loadFromFile(const String& filePath, const ostd::String& texturesSubPath)
	{
		s_error = true;
		Mesh mesh;
		loadRawData(filePath);
		if (s_rawData.size() == 0) return mesh; //TODO: Error
		serial::SerialIO reader(s_rawData, serial::SerialIO::tEndianness::BigEndian);
		int32_t meshCount = 0;
		StreamIndex addr = 0;
		//Start Reading
		reader.r_DWord(addr, meshCount);
		addr += tTypeSize::DWORD;
		if (meshCount < 1) return mesh; //TODO: Error
		meshCount = 1; //TODO: Remove this. This is a manual override to force reading only the first mesh
		for (int32_t currentMeshIndex = meshCount - 1; currentMeshIndex >= 0; currentMeshIndex--)
		{
			int32_t vertCount = 0;
			reader.r_DWord(addr, vertCount);
			addr += tTypeSize::DWORD;
			if (vertCount < 3) return mesh; //TODO: Error
			for (uint32_t v = 0; v < vertCount; v++)
			{
				tVertex vertex;
				reader.r_Float(addr, vertex.position.x);
				addr += tTypeSize::FLOAT;
				reader.r_Float(addr, vertex.position.y);
				addr += tTypeSize::FLOAT;
				reader.r_Float(addr, vertex.position.z);
				addr += tTypeSize::FLOAT;
				
				reader.r_Float(addr, vertex.normal.x);
				addr += tTypeSize::FLOAT;
				reader.r_Float(addr, vertex.normal.y);
				addr += tTypeSize::FLOAT;
				reader.r_Float(addr, vertex.normal.z);
				addr += tTypeSize::FLOAT;

				reader.r_Float(addr, vertex.texCoords.x);
				addr += tTypeSize::FLOAT;
				reader.r_Float(addr, vertex.texCoords.y);
				addr += tTypeSize::FLOAT;

				mesh.vertices.push_back(vertex);
			}
			int32_t indicesCount = 0;
			reader.r_DWord(addr, indicesCount);
			addr += tTypeSize::DWORD;
			if (indicesCount < 3) return mesh; //TODO: Error
			for (uint32_t i = 0; i < indicesCount; i++)
			{
				int32_t index = 0;
				reader.r_DWord(addr, index);
				addr += tTypeSize::DWORD;
				mesh.indices.push_back(index);
			}
			int32_t textureCount = 0;
			reader.r_DWord(addr, textureCount);
			addr += tTypeSize::DWORD;
			if (textureCount < 1) break;
			for (uint32_t t = 0; t < textureCount; t++)
			{
				int8_t tex_type = 0; //TODO: Use
				reader.r_Byte(addr, tex_type);
				addr += tTypeSize::BYTE;
				int32_t tex_path_len = 0;
				reader.r_DWord(addr, tex_path_len);
				addr += tTypeSize::DWORD;
				if (tex_path_len < 1) return mesh; //TODO: Error
				StringEditor texPath_se;
				if (texturesSubPath != "")
					texPath_se.add(texturesSubPath).add("/");
				for (uint32_t i = 0; i < tex_path_len; i++)
				{
					int8_t c = 0;
					reader.r_Byte(addr, c);
					addr += tTypeSize::BYTE;
					texPath_se.add(c);
				}
				/*std::vector<uint8_t> tex_data;
				int32_t tex_size = 0;
				reader.r_DWord(addr, tex_size);
				addr += tTypeSize::DWORD;
				if (tex_size < 4) return mesh; //TODO: Error
				for (uint32_t i = 0; i < tex_size; i++)
				{
					int8_t b = 0;
					reader.r_Byte(addr, b);
					addr += tTypeSize::BYTE;
					tex_data.push_back(b);
				}*/

				//mesh.textures.push_back(ResourceManager::loadTexture(&tex_data[0], tex_size, 0, 0));
				mesh.textures.push_back(ResourceManager::loadTexture(texPath_se.str()));
			}
		}
		s_error = false;
		//mesh.initialize();
		return mesh;
	}
	
	void OX3DLoader::loadRawData(const String& filePath)
	{
		s_rawData.clear();
		std::ifstream fin(filePath, std::ios::binary);
		if (!fin.is_open()) return; //TODO: Error
		fin.seekg(0, std::ios::end);
		size_t fileSize = fin.tellg();
		fin.seekg(0, std::ios::beg);
		s_rawData = ByteStream(fileSize, 0);
		fin.read(reinterpret_cast<char*>(&s_rawData[0]), fileSize);
	}
}
