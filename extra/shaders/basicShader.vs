#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aCol;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aNormal;

out vec2 outTexCoords;
out vec3 outNormal;
out vec3 outFragPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uNorm;

void main()
{
	gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
	outTexCoords = aTexCoords;
	outNormal = vec3(uNorm * vec4(aNormal, 1.0));
	outFragPos = vec3(uModel * vec4(aPos, 1.0));
}
