#pragma once

#include <ogfx/Window.hpp>
#include <ogfx/Camera.hpp>
#include <ogfx/RenderTarget.hpp>
#include <ogfx/Mesh.hpp>
#include <ogfx/Texture.hpp>

using namespace ostd;
using namespace ogfx;

class TestApp3D : public ostd::BaseObject
{
	public:
		TestApp3D(void);
		void handleSignal(ostd::tSignal& signal) override;	
		void gameLoop(void);

	public:
		Window window;
		FirstPersonCamera camera;

		Mesh floor;
		//TempCubeMesh cube;
		Mesh lightEmitter;

		ResourceID basicShaderID;
		ResourceID lightShaderID;
		ResourceID boxTextureID;
		ResourceID boxTextureSpecID;

		Mesh tempMesh;

		RenderTarget target;
		Mesh screenPlane;
		ResourceID screenPlaneShaderID;
		
		CubeTexture skyBox;

		const std::vector<String> skyboxPaths {
			"textures/skybox/right.jpg",
			"textures/skybox/left.jpg",
			"textures/skybox/top.jpg",
			"textures/skybox/bottom.jpg",
			"textures/skybox/front.jpg",
			"textures/skybox/back.jpg",
		};

		Mesh skyboxCube;
		ResourceID skyboxShaderID;
};