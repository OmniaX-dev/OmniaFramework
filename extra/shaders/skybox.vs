#version 420

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aCol;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aNormal;

out vec3 TexCoords;

uniform mat4 proj;
uniform mat4 view;

void main()
{
	gl_Position = proj * view * vec4(aPos, 1.0);
	TexCoords = aPos;
}
