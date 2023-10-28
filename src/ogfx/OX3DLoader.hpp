#ifndef __OX3D_LOADER_HPP__
#define __OX3D_LOADER_HPP__

#include <ostd/Types.hpp>
#include <ogfx/Mesh.hpp>

namespace ogfx
{
	class OX3DLoader
	{
		public:
			static std::vector<Mesh> loadFromFile(const ostd::String& filePath, const ostd::String& texturesSubPath = ""); //TODO: Handle more than only the first mesh
			static Mesh loadSingleMeshFromFile(const ostd::String& filePath, const ostd::String& texturesPath = "");
			static inline bool errorOccurred(void) { return s_error; }

		private:
			static void loadRawData(const ostd::String& filePath);

		private:
			inline static ostd::ByteStream s_rawData;
			inline static bool s_error = false;

	};
}

#endif
