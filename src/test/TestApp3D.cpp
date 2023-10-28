#include "TestApp3D.hpp"
#include <ogfx/OX3DLoader.hpp>
#include <ogfx/ResourceManager.hpp>
#include <ostd/Signals.hpp>
#include <ogfx/Renderer2D.hpp>

TestApp3D::TestApp3D(void)
{
	UI16Point screenSize = { 1280, 720 };

	window.create(screenSize, "OmniaX 3D Test", tContextSettings());
	window.centerMouse().hideMouseCursor();
	camera.create(window);

	tempMesh = OX3DLoader::loadSingleMeshFromFile("models/monkey2.ox3d", "textures");
	screenPlane = Mesh::newPlaneMesh();
	floor = Mesh::newCubeMesh();
	lightEmitter = Mesh::newCubeMesh();
	skyboxCube = Mesh::newCubeMesh();
	if (OX3DLoader::errorOccurred())
	{
		OX_ERROR("OX3DLoader failed to load model.");
		exit(1);
		return;
	}

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //TODO: Add option for wireframe
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);  


	basicShaderID = ResourceManager::loadShader("basicShader", false);
	lightShaderID = ResourceManager::loadShader("lightShader", false);
	screenPlaneShaderID = ResourceManager::loadShader("screenPlane", false);
	skyboxShaderID = ResourceManager::loadShader("skybox", false);
	boxTextureID = ResourceManager::loadTexture("textures/container2.png");
	boxTextureSpecID = ResourceManager::loadTexture("textures/container2_specular.png");
	
	auto& basicShader = ResourceManager::getShader(basicShaderID);
	basicShader.registerUniform("material.diffuse");
	basicShader.registerUniform("material.specular");
	basicShader.registerUniform("material.shininess");
	
	basicShader.registerUniform("light.position");
	basicShader.registerUniform("light.direction");
	basicShader.registerUniform("light.cutOff");
	basicShader.registerUniform("light.outerCutOff");
	basicShader.registerUniform("light.ambient");
	basicShader.registerUniform("light.diffuse");
	basicShader.registerUniform("light.specular");
	basicShader.registerUniform("light.linear");
	basicShader.registerUniform("light.constant");
	basicShader.registerUniform("light.quadratic");
	basicShader.registerUniform("light.type");

	skyBox.create(skyboxPaths);

	validate();
	enableSignals();
	connectSignal(tBuiltinSignals::OnGuiEvent);
	window.initialize();
	tempMesh.initialize();
	screenPlane.initialize();
	target.create(screenSize.x, screenSize.y);

	floor.initialize();
	floor.getTransform().rotate({ -90.0f, 0.0f, 0.0f }).scale({ 300.0f, 0.01f, 300.0f }).translate({ 0.0f, -3.0f, 0.0f }).apply();
	lightEmitter.initialize();
	lightEmitter.getTransform().scale({ 0.3f, 0.3f, 0.3f }).translate({ 10.0f, 10.0f, 2.0f }).apply();
	skyboxCube.initialize();
	skyboxCube.getTransform().scale({ 400.0f, 400.0f, 400.0f }).apply();
}

void TestApp3D::handleSignal(ostd::tSignal &signal)
{
	if (signal.ID == tBuiltinSignals::OnGuiEvent)
	{
		Event& evt = (Event&)signal.userData;
		sf::Event& event = evt.sf();
		if (event.type == sf::Event::Closed)
		{
			window.close();
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Escape)
				window.close();
			else if (event.key.code == sf::Keyboard::W)
				camera.KeyPressed(FirstPersonCamera::KeyForward);
			else if (event.key.code == sf::Keyboard::A)
				camera.KeyPressed(FirstPersonCamera::KeyLeft);
			else if (event.key.code == sf::Keyboard::S)
				camera.KeyPressed(FirstPersonCamera::KeyBackwards);
			else if (event.key.code == sf::Keyboard::D)
				camera.KeyPressed(FirstPersonCamera::KeyRight);
			else if (event.key.code == sf::Keyboard::Space)
				camera.KeyPressed(FirstPersonCamera::KeyUp);
			else if (event.key.code == sf::Keyboard::LShift)
				camera.KeyPressed(FirstPersonCamera::KeyDown);
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			if (event.key.code == sf::Keyboard::W)
				camera.keyReleased(FirstPersonCamera::KeyForward);
			else if (event.key.code == sf::Keyboard::A)
				camera.keyReleased(FirstPersonCamera::KeyLeft);
			else if (event.key.code == sf::Keyboard::S)
				camera.keyReleased(FirstPersonCamera::KeyBackwards);
			else if (event.key.code == sf::Keyboard::D)
				camera.keyReleased(FirstPersonCamera::KeyRight);
			else if (event.key.code == sf::Keyboard::Space)
				camera.keyReleased(FirstPersonCamera::KeyUp);
			else if (event.key.code == sf::Keyboard::LShift)
				camera.keyReleased(FirstPersonCamera::KeyDown);
		}
		else if (event.type == sf::Event::Resized)
		{
			// adjust the viewport when the window is resized
			glViewport(0, 0, event.size.width, event.size.height);
			window.initialize();
			camera.updateProjectionMatrix();
		}
		else if (event.type == sf::Event::MouseMoved)
		{
			if (!window.isMouseMoveEnabled())
			{
				window.disableMouseMove(false);
				return;
			}
			camera.mouseMoved((float)event.mouseMove.x, (float)event.mouseMove.y);
		}
	}
}

void TestApp3D::gameLoop(void)
{
	auto& basicShader = ResourceManager::getShader(basicShaderID);
	if (basicShader.isInvalid()) exit(1);
	auto& lightShader = ResourceManager::getShader(lightShaderID);
	if (lightShader.isInvalid()) exit(2);
	auto& boxTexture = ResourceManager::getTexture(boxTextureID);
	auto& boxTextureSpec = ResourceManager::getTexture(boxTextureSpecID);

	auto& tempMeshTexture = ResourceManager::getTexture(tempMesh.textures[0]);

	auto& screenPlaneShader = ResourceManager::getShader(screenPlaneShaderID);

	auto& skyboxShader = ResourceManager::getShader(skyboxShaderID);
	
	Color::FloatCol skyCol { 10.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f };

	while (window.isRunning())
	{
		window.handleEvents();
		camera.update();
		target.bind();
		glEnable(GL_DEPTH_TEST);
		glClearColor(skyCol.r, skyCol.g, skyCol.b, skyCol.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		skyboxShader.bind();
		skyboxShader.updateUniform_mat4f("view", glm::mat4(glm::mat3(camera.getViewMat())));
		skyboxShader.updateUniform_mat4f("proj", camera.getProjMat());
		skyboxShader.updateUniform_i("skybox", 0);
		skyboxCube.bind();
		skyboxCube.draw();
		glDepthMask(GL_TRUE);

		tempMesh.getTransform().rotate({ 0.0f, 0.0f, 0.5f }).apply();
		

		tempMeshTexture.bind(0);
		boxTextureSpec.bind(1);
		basicShader.bind();

		basicShader.updateUniform_i("material.diffuse", 0);
		basicShader.updateUniform_i("material.specular", 1);
		basicShader.updateUniform_f("material.shininess", 64.0f);
		
		basicShader.updateUniform_vec3f("light.ambient", { 0.07f, 0.07f, 0.07f });
		basicShader.updateUniform_vec3f("light.diffuse", { 0.5f, 0.5f, 0.5f });
		basicShader.updateUniform_vec3f("light.specular", { 1.0f, 1.0f, 1.0f });
		basicShader.updateUniform_vec3f("light.position", lightEmitter.getTransform().getTranslation());
		basicShader.updateUniform_vec3f("light.direction", camera.getFront());
		basicShader.updateUniform_f("light.constant", 1.0f);
		basicShader.updateUniform_f("light.linear", 0.009f);
		basicShader.updateUniform_f("light.quadratic", 0.032f);
		basicShader.updateUniform_i("light.type", 2);
		basicShader.updateUniform_f("light.cutOff", glm::cos(glm::radians(12.5f)));
		basicShader.updateUniform_f("light.outerCutOff", glm::cos(glm::radians(17.5f)));

		basicShader.updateUniform_vec3f("uViewPosition", camera.getPosition());
		basicShader.updateUniform_mat4f("uModel", tempMesh.getTransform().getMatrix());
		basicShader.updateUniform_mat4f("uView", camera.getViewMat());
		basicShader.updateUniform_mat4f("uProj", camera.getProjMat());
		basicShader.updateUniform_mat4f("uNorm", tempMesh.getTransform().getNormalMatrix());
		tempMesh.bind();
		tempMesh.draw();

		basicShader.updateUniform_f("material.shininess", 4.0f);
		basicShader.updateUniform_mat4f("uModel", floor.getTransform().getMatrix());
		basicShader.updateUniform_mat4f("uNorm", floor.getTransform().getNormalMatrix());
		boxTexture.bind();
		floor.bind();
		floor.draw();
		
		glDisable(GL_CULL_FACE);  
		lightShader.bind();
		lightShader.updateUniform_mat4f("uModel", lightEmitter.getTransform().getMatrix());
		lightShader.updateUniform_mat4f("uView", camera.getViewMat());
		lightShader.updateUniform_mat4f("uProj", camera.getProjMat());
		lightEmitter.draw();


		target.unbind();

		screenPlaneShader.bind();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);  
		ResourceManager::getTexture(target.getResourceID()).bind(0);
		screenPlaneShader.updateUniform_i("screenTexture", 0);
		glClear(GL_COLOR_BUFFER_BIT);
		screenPlane.bind();
		screenPlane.draw();

		window.renderFrame();
	}
}