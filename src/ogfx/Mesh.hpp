#ifndef __TEMP_HPP__
#define __TEMP_HPP__

#include <ogfx/DataStructures.hpp>
#include <vector>

namespace ogfx
{
	class Mesh
	{
		public:
			inline Mesh(void) {  }
			void initialize(void);
			inline Transform3D& getTransform(void) { return transform; }
			void bind(void);
			void unbind(void);
			void draw(void);

			static Mesh newPlaneMesh(bool auto_initialize = false);
			static Mesh newCubeMesh(bool auto_initialize = false);

		private:
			VertexArray vao { false };
			VertexBuffer vbo;
			ElementBuffer ebo;
			Transform3D transform;

		public:
			std::vector<uint32_t> indices;
			std::vector<tVertex> vertices;
			std::vector<ResourceID> textures;
	};

}

#endif
