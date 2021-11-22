#include "Renderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CubeRobot.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "../nclgl/Material.h"
#include "../nclgl/WaveMaterial.h"
#include "../nclgl/TerrainMaterial.h"
#include  <algorithm>                //For  std::sort ...
const int POST_PASSES = 10;
const int LIGHT_NUM = 32;
Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	root = new SceneNode();

	quad = Mesh::GenerateQuad();
	quad2 = Mesh::GenerateQuad();

	// Load in shaders and textures

	shaders.emplace_back(new Shader("bumpVertex.glsl", "bumpFragment.glsl"));
	shaders.emplace_back(new Shader("skyboxVertex.glsl", "skyboxFragment.glsl"));
	shaders.emplace_back(new Shader("reflectVertex.glsl", "reflectFragment.glsl"));

	pointlightShader = new Shader("pointlightvert.glsl",
		"pointlightfrag.glsl");
	combineShader = new Shader("combinevert.glsl",
		"combinefrag.glsl");

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

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"dudv.png", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"tileable_grass_00.png", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"grass/grass01_n.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"Sand2.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	textures.emplace_back(SOIL_load_OGL_texture(
		TEXTUREDIR"grass/grass01_n.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	for (auto it = shaders.begin(); it != shaders.end(); it++) {
		if (!(*it)->LoadSuccess()) return;
	}
	for (auto it = textures.begin(); it != textures.end(); it++) {
		if (!(*it)) {
			return;
		}
	}

	SetTextureRepeating(textures[0], true);
	SetTextureRepeating(textures[1], true);
	SetTextureRepeating(textures[2], true);
	SetTextureRepeating(textures[3], true);
	SetTextureRepeating(textures[4], true);
	SetTextureRepeating(textures[5], true);
	SetTextureRepeating(textures[6], true);
	SetTextureRepeating(textures[7], true);
	SetTextureRepeating(textures[8], true);
	SetTextureRepeating(textures[9], true);

	// Create height map
	SceneNode* heightMap = new SceneNode();
	TerrainMaterial* heightMat = new TerrainMaterial();
	heightMat->SetShader(shaders[0]);

	heightMat->SetSandTex(textures[8]);
	heightMat->SetSandNormal(textures[9]);
	heightMat->SetGrassTex(textures[6]);
	heightMat->SetGrassNormal(textures[7]);
	heightMat->SetRockTex(textures[0]);
	heightMat->SetRockNormal(textures[1]);

	HeightMap* heightMapMesh = new HeightMap(TEXTUREDIR"noise.png");
	heightMap->SetMesh(heightMapMesh);
	heightMap->SetModelScale(Vector3(1, 1, 1));
	heightMap->SetMaterial(heightMat);
	root->AddChild(heightMap);
	materials.emplace_back(heightMat);
	heightmapSize = heightMapMesh->GetHeightmapSize();

	// Create water
	SceneNode* water = new SceneNode();
	WaveMaterial* waterMat = new WaveMaterial();
	waterMat->SetTexture(textures[3]);
	waterMat->SetShader(shaders[2]);
	waterMat->SetCubeMap(textures[2]);
	waterMat->SetBump(textures[4]);
	waterMat->SetDuDvTex(textures[5]);
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

	pointLights = new Light[LIGHT_NUM];

	for (int i = 0; i < LIGHT_NUM; ++i) {
		Light& l = pointLights[i];
		l.SetPosition(Vector3(rand() % (int)heightmapSize.x,
			250.0f,
			rand() % (int)heightmapSize.z));

		/*l.SetColour(Vector4(0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			1));*/
		l.SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		l.SetRadius(250.0f + (rand() % 250));
		l.SetRadius(1000.0f);
	}

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	// Refraction and reflection buffers
	glGenTextures(1, &refractionBufferTex);
	glBindTexture(GL_TEXTURE_2D, refractionBufferTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenTextures(1, &reflectionBufferTex);
	glBindTexture(GL_TEXTURE_2D, reflectionBufferTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &refractionFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, refractionBufferTex, 0);

	glGenFramebuffers(1, &reflectionFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, reflectionBufferTex, 0);

	// Depth buffer ( should be able to be combined with refraction)
	glGenTextures(1, &waterDepthTex);
	glBindTexture(GL_TEXTURE_2D, waterDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, waterDepthTex, 0);

	//We can check FBO attachment success using this command!
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !refractionBufferTex || !reflectionBufferTex) {
		return;
	}

	refractionClipPlane = Vector4(0, -1, 0, water->GetTransform().GetPositionVector().y + waterMat->GetAmplitude());
	reflectionClipPlane = Vector4(0, 1, 0, -(water->GetTransform().GetPositionVector().y));
	waterMat->SetReflectionTex(reflectionBufferTex);
	waterMat->SetRefractionTex(refractionBufferTex);

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0 ,
		GL_COLOR_ATTACHMENT1
	};

	// Generate our scene depth texture ...
	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);

	//And now attach them to our FBOs
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	glBindBuffer(GL_BUFFER, 0);
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
	for (auto it = materials.begin(); it != materials.end(); it++) {
		delete *it;
	}
	delete quad;

	delete pointlightShader;
	delete combineShader;
	delete[] pointLights;

	glDeleteTextures(1, &refractionBufferTex);
	glDeleteTextures(1, &reflectionBufferTex);
	glDeleteTextures(1, &waterDepthTex);
	glDeleteFramebuffers(1, &refractionFBO);
	glDeleteFramebuffers(1, &reflectionFBO);
	glDeleteFramebuffers(1, &depthFBO);

	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);

	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0,
		format, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
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
	glEnable(GL_CLIP_DISTANCE0);
	glEnable(GL_CULL_FACE);
	clipPlane = refractionClipPlane;
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	for (const auto& i : nodeList) {
		DrawNode(i);
	}

	clipPlane = reflectionClipPlane;
	float distance = 2 * (camera->GetPosition().y - (refractionClipPlane.w - 2.5));
	camera->SetPosition(Vector3(camera->GetPosition().x,camera->GetPosition().y - distance, camera->GetPosition().z));
	camera->SetPitch(-camera->GetPitch());
	viewMatrix = camera->BuildViewMatrix();

	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	camera->SetPosition(Vector3(camera->GetPosition().x, camera->GetPosition().y + distance, camera->GetPosition().z));
	camera->SetPitch(-camera->GetPitch());
	viewMatrix = camera->BuildViewMatrix();

	//glDisable(GL_CULL_FACE);
	glDisable(GL_CLIP_DISTANCE0);

	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	for (const auto& i : nodeList) {
		DrawNode(i);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//DrawSkybox();

	FillBuffers();
	DrawPointLights();
	CombineBuffers();



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

			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D, waterDepthTex);
			glUniform1i(glGetUniformLocation(shader->GetProgram(), "depthTex"), 15);

			glUniform4fv(glGetUniformLocation(shader->GetProgram(), "plane"), 1, (float*)&clipPlane);

			modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			normalMatrix = modelMatrix.Inverse().GetTransposedRotation();

			UpdateShaderMatrices();
			glUniformMatrix3fv(glGetUniformLocation(shader->GetProgram(), "normalMatrix"), 1, false, normalMatrix.values);

			n->Draw(*this);
		}
	}
}

void  Renderer::RenderScene() {
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

void Renderer::FillBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	for (const auto& i : nodeList) {
		DrawNode(i);
	}

	/*for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawPointLights() {
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(pointlightShader);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(
		pointlightShader->GetProgram(), "depthTex"), 20);
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(
		pointlightShader->GetProgram(), "normTex"), 21);
	glActiveTexture(GL_TEXTURE21);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(pointlightShader->GetProgram(),
		"cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform2f(glGetUniformLocation(pointlightShader->GetProgram(),
		"pixelSize"), 1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(
		pointlightShader->GetProgram(), "inverseProjView"),
		1, false, invViewProj.values);

	UpdateShaderMatrices();
	for (int i = 0; i < LIGHT_NUM; ++i) {
		Light& l = pointLights[i];
		SetShaderLight(l);
		sphere->Draw();

	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Renderer::CombineBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDepthMask(GL_FALSE);

	BindShader(combineShader);
	//modelMatrix.ToIdentity();
	//viewMatrix.ToIdentity();
	//projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseTex"), 22);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseLight"),23);
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "specularLight"), 24);
	glActiveTexture(GL_TEXTURE24);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	quad2->Draw();
	glDepthMask(GL_TRUE);
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}
