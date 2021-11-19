#include "Renderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CubeRobot.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "../nclgl/Material.h"
#include "../nclgl/WaveMaterial.h"
#include  <algorithm>                //For  std::sort ...
const int POST_PASSES = 10;
Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	root = new SceneNode();

	quad = Mesh::GenerateQuad();

	// Load in shaders and textures

	processShader = new Shader("TexturedVertex.glsl", "processfrag.glsl");
	sceneShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	shaders.emplace_back(new Shader("bumpVertex.glsl", "bumpFragment.glsl"));
	shaders.emplace_back(new Shader("skyboxVertex.glsl", "skyboxFragment.glsl"));
	shaders.emplace_back(new Shader("reflectVertex.glsl", "reflectFragment.glsl"));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_cubemap(
		TEXTUREDIR"skybox/right.png", TEXTUREDIR"skybox/left.png",
		TEXTUREDIR"skybox/top.png", TEXTUREDIR"rusted_south.jpg",
		TEXTUREDIR"skybox/back.png", TEXTUREDIR"skybox/front.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"waterbump.png", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	if (!processShader->LoadSuccess() || !sceneShader->LoadSuccess()) {
		return;
	}
	for (auto it = shaders.begin(); it != shaders.end(); it++) {
		if (!(*it)->LoadSuccess()) return;
	}
	for (auto it = textures.begin(); it != textures.end(); it++) {
		if (!(*it)) return;
	}

	SetTextureRepeating(textures[0], true);
	SetTextureRepeating(textures[1], true);
	SetTextureRepeating(textures[2], true);
	SetTextureRepeating(textures[3], true);
	SetTextureRepeating(textures[4], true);

	// Create height map
	SceneNode* heightMap = new SceneNode();
	Material* heightMat = new Material();
	heightMat->SetShader(shaders[0]);
	heightMat->SetTexture(textures[0]);
	heightMat->SetBump(textures[1]);
	HeightMap* heightMapMesh = new HeightMap(TEXTUREDIR"noise.png");
	heightMap->SetMesh(heightMapMesh);
	heightMap->SetModelScale(Vector3(1, 1, 1));
	heightMap->SetMaterial(heightMat);
	root->AddChild(heightMap);
	materials.emplace_back(heightMat);
	heightmapSize = heightMapMesh->GetHeightmapSize();

	// Create water
	SceneNode* water = new SceneNode();
	Material* waterMat = new WaveMaterial();
	waterMat->SetTexture(textures[3]);
	waterMat->SetShader(shaders[2]);
	waterMat->SetCubeMap(textures[2]);
	waterMat->SetBump(textures[4]);
	HeightMap* waterMapMesh = new HeightMap();
	water->SetMesh(waterMapMesh);
	water->SetColour(Vector4(1.0, 1.0, 1.0, 0.5));
	water->SetTransform(Matrix4::Translation(Vector3(0, 150, 0)));
	water->SetMaterial(waterMat);

	root->AddChild(water);
	materials.emplace_back(waterMat);

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
	delete processShader;
	delete sceneShader;
	for (auto it = shaders.begin(); it != shaders.end(); it++) {
		delete* it;
	}
	for (auto it = textures.begin(); it != textures.end(); it++) {
		glDeleteTextures(1, &(*it));
	}
	for (auto it = materials.begin(); it != materials.end(); it++) {
		delete *it;
	}
	delete quad;


	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
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
		if (n->GetMaterial()) {
			Material* material = n->GetMaterial();
			Shader* shader = (Shader*)(material->GetShader());
			BindShader(shader);

			SetShaderLight(*light);
			material->PassShaderUniforms();

			glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());
			glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());
			glUniform1f(glGetUniformLocation(shader->GetProgram(), "sceneTime"), sceneTime);


			modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			normalMatrix = modelMatrix.Inverse().GetTransposedRotation();

			UpdateShaderMatrices();
			glUniformMatrix3fv(glGetUniformLocation(shader->GetProgram(), "normalMatrix"), 1, false, normalMatrix.values);

			n->Draw(*this);
		}
	}
}

void  Renderer::RenderScene() {
	DrawSkybox();
	BuildNodeLists(root);
	SortNodeLists();
	DrawNodes();
	ClearNodeLists();
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawSkybox() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDepthMask(GL_FALSE);


	BindShader(shaders[1]);

	glUniform1i(glGetUniformLocation(
		shaders[1]->GetProgram(), "cubeTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[2]);

	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}
