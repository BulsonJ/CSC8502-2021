#include "Renderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CubeRobot.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include  <algorithm>                //For  std::sort ...
Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();

	// Load in shaders and textures

	shaders.emplace_back(new Shader("bumpVertex.glsl", "bumpFragment.glsl"));
	shaders.emplace_back(new Shader("reflectVertex.glsl", "reflectFragment.glsl"));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	for (auto it = shaders.begin(); it != shaders.end(); it++) {
		if (!(*it)->LoadSuccess()) return;
	}
	for (auto it = textures.begin(); it != textures.end(); it++) {
		if (!(*it)) return;
	}

	
	SetTextureRepeating(textures[0], true);
	SetTextureRepeating(textures[1], true);
	SetTextureRepeating(textures[2], true);

	root = new SceneNode();

	// Create height map
	SceneNode* heightMap = new SceneNode();
	HeightMap* heightMapMesh = new HeightMap(TEXTUREDIR"noise.png");
	heightMap->SetMesh(heightMapMesh);
	heightMap->SetShaderOverall(heightMap,shaders[0]);
	heightMap->SetTexture(textures[0]);
	heightMap->SetBump(textures[1]);
	heightMap->SetModelScale(Vector3(1, 1, 1));
	root->AddChild(heightMap);
	heightmapSize = heightMapMesh->GetHeightmapSize();

	// Create water
	SceneNode* water = new SceneNode();
	HeightMap* waterMapMesh = new HeightMap();
	water->SetMesh(waterMapMesh);
	water->SetTexture(textures[2]);
	water->SetShaderOverall(water, shaders[1]);
	water->SetUseLight(false);
	water->SetColour(Vector4(1.0, 1.0, 1.0, 0.5));
	water->SetTransform(Matrix4::Translation(Vector3(0, 150, 0)));
	root->AddChild(water);


	camera = new Camera(-45.0f, 0.0f,
		heightmapSize * Vector3(0.5f, 5.0f, 0.5f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f),
		Vector4(1, 1, 1, 1), heightmapSize.x);


	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	sceneTime = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	delete  root;
	delete  camera;
	for (auto it = shaders.begin(); it != shaders.end(); it++) {
		delete* it;
	}
	for (auto it = textures.begin(); it != textures.end(); it++) {
		glDeleteTextures(1, &(*it));
	}
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	sceneTime += dt;

	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);
}

void   Renderer::BuildNodeLists(SceneNode* from) {
	//if (frameFrustum.InsideFrustum(*from)) {
	if (true) {
		Vector3  dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));
		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}
	for (vector <SceneNode*>::const_iterator i =
		from->GetChildIteratorStart();
		i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(),    //note  the r!
		transparentNodeList.rend(),      //note  the r!
		SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),
		nodeList.end(),
		SceneNode::CompareByCameraDistance);
}

void   Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {

	if (n->GetMesh()) {
		if (n->GetShader()) {
			Shader* shader = (Shader*)(n->GetShader());

			BindShader(shader);

			// Get texture of scene node, if scene node has texture it will be bound and uniform set to 1,
			// otherwise set to 0
			if (n->GetTexture()) {
				GLuint texture = n->GetTexture();
				glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);

				glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), 1);
			}
			else {
				glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), 0);
			}


			// Light
			if (n->GetUseLight()) {
				SetShaderLight(*light);
			}

			if (n->GetBump()) {
				GLuint bumptexture = n->GetBump();
				glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, bumptexture);
			}

			glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

			glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());

			// Custom uniforms
			glUniform1f(glGetUniformLocation(shader->GetProgram(), "sceneTime"), sceneTime);

			if (n->GetModifyModelMatrix()) {
				modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			}
			else {
				modelMatrix.ToIdentity(); //New!
				textureMatrix.ToIdentity(); //New!
			}

			UpdateShaderMatrices();

			n->Draw(*this);
		}
	}
}

void  Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawNodes();
	ClearNodeLists();
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}
