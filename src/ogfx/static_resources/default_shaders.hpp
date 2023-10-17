#ifndef __DEFAULT_SHADERS_HPP__
#define __DEFAULT_SHADERS_HPP__

#include <ostd/Types.hpp>

namespace oxres
{
	const ostd::String defaultShader_vert =
		"#version 420 core\n"

		"layout(location = 0) in vec3    a_position;\n"
		"layout(location = 1) in vec4    a_color;\n"
		"layout(location = 2) in vec2    a_texCoords;\n"
		"layout(location = 3) in float   a_texIndex;\n"

		"out vec4     v_vertexColor;\n"
		"out vec2     v_texCoords;\n"
		"out float    v_texIndex;\n"

		"uniform mat4     u_viewProjMatrix;\n"

		"void main()\n"
		"{\n"
			"gl_Position = u_viewProjMatrix * vec4(a_position, 1.0);\n"
			"v_vertexColor = a_color;\n"
			"v_texCoords = a_texCoords;\n"
			"v_texIndex = a_texIndex;\n"
		"}\n"
	;
	const ostd::String defaultShader_frag =
		"#version 420 core\n"

		"out vec4 out_FragColor;\n"
		"uniform sampler2D u_textures[16];\n"

		"in vec4 v_vertexColor;\n"
		"in vec2 v_texCoords;\n"
		"in float v_texIndex;\n"

		"void main()\n"
		"{\n"
			"int iTexID = int(v_texIndex);\n"
			"vec4 color = texture(u_textures[iTexID], v_texCoords) * v_vertexColor;\n"
			"out_FragColor = color;"
		"}\n"
	;



	const ostd::String defaultBlendShader_vert =
		"#version 420 core\n"

		"layout(location = 0) in vec3    a_position;\n"
		"layout(location = 1) in vec4    a_color;\n"
		"layout(location = 2) in vec2    a_texCoords;\n"
		"layout(location = 3) in float   a_texIndex;\n"

		"out vec2     v_texCoords;\n"

		"uniform mat4     u_viewProjMatrix;\n"

		"void main()\n"
		"{\n"
			"gl_Position = u_viewProjMatrix * vec4(a_position, 1.0);\n"
			"v_texCoords = a_texCoords;\n"
		"}\n"
	;
	const ostd::String defaultBlendShader_frag =
		"#version 420 core\n"

		"out vec4 out_FragColor;\n"
		"uniform sampler2D  u_textures[16];\n"
		"uniform int u_blendModes[14];\n"
		"uniform int u_layerCount;\n"
		"in vec2 v_texCoords;\n"

		"const int BMOverride = 0;\n"
		"const int BMTransparent = 1;\n"
		"const int BMMultiply = 2;\n"
		"const int BMAdd = 3;\n"
		"const int BMSubtract = 4;\n"
		"const int BMNormal = 5;\n"

		"void main()\n"
		"{\n"
			"vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
			"for (int i = 0; i < u_layerCount; i++)\n"
			"{\n"
				"int ti = i + 1;\n"
				"int bm = u_blendModes[i];\n"
				"if (bm == BMOverride)\n"
					"color = texture(u_textures[ti], v_texCoords);\n"
				"else if (bm == BMTransparent)\n"
					"continue;\n"
				"else if (bm == BMMultiply)\n"
					"color *= texture(u_textures[ti], v_texCoords);\n"
				"else if (bm == BMAdd)\n"
					"color += texture(u_textures[ti], v_texCoords);\n"
				"else if (bm == BMSubtract)\n"
					"color -= texture(u_textures[ti], v_texCoords);\n"
				"else if (bm == BMNormal)\n"
				"{\n"
					"vec4 col = texture(u_textures[ti], v_texCoords);\n"
					"color = vec4(col.rgb * col.a + color.rgb * (1.0 - col.a), 1.0);\n"
				"}\n"
			"}\n"
			"out_FragColor = color;\n"
		"}\n"
	;
}

#endif