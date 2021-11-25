#include "Renderer.h"
#include "../nclgl/Matrix4.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CubeRobot.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "../nclgl/SpotLight.h"
#include "../nclgl/DirectionalLight.h"
#include "../nclgl/Material.h"
#include "../nclgl/WaveMaterial.h"
#include "../nclgl/TerrainMaterial.h"
#include "../nclgl/PointLight.h"
#include  <algorithm>                //For  std::sort ...
#define SHADOWSIZE 2048
const int POINT_LIGHT_NUM = 1;
const int SPOT_LIGHT_NUM = 5;
Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	root = new SceneNode();
	cone = Mesh::LoadFromMeshFile("Cone.msh");
	quad = Mesh::GenerateQuad();

	// Load in shaders and textures

	shaders.emplace_back(new Shader("bumpVertex.glsl", "bumpFragment.glsl"));
	shaders.emplace_back(new Shader("skyboxVertex.glsl", "skyboxFragment.glsl"));
	shaders.emplace_back(new Shader("reflectVertex.glsl", "reflectFragment.glsl"));

	directionallightShader = new Shader("Deferred/directionallightVert.glsl","Deferred/directionallightFrag.glsl");
	pointlightShader = new Shader("Deferred/pointlightvert.glsl","Deferred/pointlightfrag.glsl");
	spotlightShader = new Shader("Deferred/spotlightVert.glsl", "Deferred/spotlightFrag.glsl");
	combineShader = new Shader("Deferred/combinevert.glsl","Deferred/combinefrag.glsl");

	shaders.emplace_back(combineShader);
	shaders.emplace_back(directionallightShader);
	shaders.emplace_back(pointlightShader);
	shaders.emplace_back(spotlightShader);

	Shader* basicShader = new Shader("TexturedVertex.glsl","TexturedFragment.glsl");
	shaders.emplace_back(basicShader);

	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	shadowCubeShader = new Shader("shadowCubeVert.glsl","shadowCubeFrag.glsl", "shadowCube.glsl");
	shaders.emplace_back(shadowShader);
	shaders.emplace_back(shadowCubeShader);


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

	//textures.emplace_back(SOIL_load_OGL_single_cubemap(TEXTUREDIR"night.png","NSWEUD", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0));

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

	HeightMap* heightMapMesh = new HeightMap(TEXTUREDIR"noise2.png");
	heightMap->SetMesh(heightMapMesh);
	heightMap->SetModelScale(Vector3(1, 1, 1));
	heightMap->SetMaterial(heightMat);
	root->AddChild(heightMap);
	materials.emplace_back(heightMat);
	heightmapSize = heightMapMesh->GetHeightmapSize();
	heightMap->SetBoundingRadius(heightmapSize.x);

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
	water->SetBoundingRadius(heightmapSize.x);

	root->AddChild(water);
	materials.emplace_back(waterMat);

	camera = new Camera(-45.0f, 0.0f,heightmapSize * Vector3(0.5f, 5.0f, 0.5f));

	directionalLight = new DirectionalLight(Vector3(-1, 0, 0), Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0);
	directionalLight->CreateShadowFBO();
	currentAngle = 0.0f;

	pointLights = new PointLight[POINT_LIGHT_NUM];

	for (int i = 0; i < POINT_LIGHT_NUM; ++i) {
		PointLight& l = pointLights[i];
		l.SetPosition(Vector3(rand() % (int)heightmapSize.x,
			350.0f,
			rand() % (int)heightmapSize.z));

		/*l.SetColour(Vector4(0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			1));*/
		l.SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		l.SetRadius(250.0f + (rand() % 250));
		l.SetRadius(1000.0f);
		l.CreateShadowFBO();
	}

	pointLights->SetPosition(
		Vector3(heightmapSize.x/2,
		500.0f,
		heightmapSize.z/2));

	spotLights = new SpotLight[SPOT_LIGHT_NUM];

	for (int i = 0; i < SPOT_LIGHT_NUM; ++i) {
		SpotLight& l = spotLights[i];
		l.SetPosition(Vector3(rand() % (int)heightmapSize.x,
			225.0f,
			rand() % (int)heightmapSize.z));

		l.SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		l.SetRadius(45.0f);
		l.SetDirection(Vector3(1, 0, 0));
		l.CreateShadowFBO();
	}

	
	Material* basic = new Material();
	basic->SetShader(basicShader);
	basic->SetTexture(textures[0]);
	SceneNode* test = new SceneNode();
	test->SetMesh(sphere);
	test->SetMaterial(basic);
	test->SetTransform(Matrix4::Translation((heightmapSize/2) + Vector3(0,125,0)));
	test->SetModelScale(Vector3(10, 10, 10));
	root->AddChild(test);

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
		GL_COLOR_ATTACHMENT0,
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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	delete directionalLight;
	delete[] pointLights;
	delete[] spotLights;

	glDeleteTextures(1, &refractionBufferTex);
	glDeleteTextures(1, &reflectionBufferTex);
	glDeleteFramebuffers(1, &refractionFBO);
	glDeleteFramebuffers(1, &reflectionFBO);

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


	float angleChange = dt * 10.0f;
	currentAngle += angleChange;
	if (currentAngle > 270) {
		currentAngle -= 360;
	}
	Matrix4 rotMatrix = Matrix4::Rotation(angleChange, Vector3(0, 0, 1));
	directionalLight->SetPosition(rotMatrix * directionalLight->GetPosition());

	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
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

void Renderer::DrawNode(SceneNode* n) {

	if (n->GetMesh()) {
		if (n->GetMaterial()) {
			Material* material = n->GetMaterial();
			Shader* shader = (Shader*)(material->GetShader());
			BindShader(shader);

			SetShaderLight(*directionalLight);
			SetShaderLights(shader);
			material->PassShaderUniforms();

			glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());
			glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());
			glUniform1f(glGetUniformLocation(shader->GetProgram(), "sceneTime"), sceneTime);

			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
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

void Renderer::SetShaderLights(Shader* shader) {

	Vector4 lightColour[POINT_LIGHT_NUM];
	Vector3 lightPos[POINT_LIGHT_NUM];
	float lightRadius[POINT_LIGHT_NUM];

	for (int i = 0; i < POINT_LIGHT_NUM; ++i) {
		if (pointLights + i == NULL) continue;

		lightRadius[i] = 0;
	}

	for (int i = 0; i < POINT_LIGHT_NUM; ++i) {
		if (pointLights + i == NULL) continue;

		Light& l = pointLights[i];
		lightColour[i] = l.GetColour();
		lightPos[i] = l.GetPosition();
		lightRadius[i] = l.GetRadius();
	}

	GLuint loc = glGetUniformLocation(shader->GetProgram(), "pointLights_lightColour");
	glUniform4fv(loc, POINT_LIGHT_NUM, (float*)& lightColour);
	loc = glGetUniformLocation(shader->GetProgram(), "pointLights_lightPos");
	glUniform3fv(loc, POINT_LIGHT_NUM, (float*)& lightPos);
	loc = glGetUniformLocation(shader->GetProgram(), "pointLights_lightRadius");
	glUniform1fv(loc, POINT_LIGHT_NUM, lightRadius);

	Vector4 spotlightColour[SPOT_LIGHT_NUM];
	Vector3 spotlightPos[SPOT_LIGHT_NUM];
	Vector3 spotlightDirection[SPOT_LIGHT_NUM];
	float spotlightCutoff[SPOT_LIGHT_NUM];

	for (int i = 0; i < POINT_LIGHT_NUM; ++i) {
		if (pointLights + i == NULL) continue;

		spotlightCutoff[i] = 0;
	}

	for (int i = 0; i < SPOT_LIGHT_NUM; ++i) {
		if (spotLights + i == NULL) continue;

		SpotLight& l = spotLights[i];
		spotlightColour[i] = l.GetColour();
		spotlightPos[i] = l.GetPosition();
		spotlightDirection[i] = l.GetDirection();
		spotlightCutoff[i] = cos(l.GetRadius() * (PI / 180));
	}

	loc = glGetUniformLocation(shader->GetProgram(), "spotLights_lightColour");
	glUniform4fv(loc, SPOT_LIGHT_NUM, (float*)& spotlightColour);
	loc = glGetUniformLocation(shader->GetProgram(), "spotLights_lightPos");
	glUniform3fv(loc, SPOT_LIGHT_NUM, (float*)& spotlightPos);
	loc = glGetUniformLocation(shader->GetProgram(), "spotLights_lightDirection");
	glUniform3fv(loc, SPOT_LIGHT_NUM, (float*)& spotlightDirection);
	loc = glGetUniformLocation(shader->GetProgram(), "spotLights_lightCutoff");
	glUniform1fv(loc, POINT_LIGHT_NUM, spotlightCutoff);

}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();
	DrawShadowScene();
	GenerateRefractionBuffer();
	GenerateReflectionBuffer();
	FillBuffers();
	DeferredLighting();
	CombineBuffers();
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

	glUniform1f(glGetUniformLocation(shaders[1]->GetProgram(), "dayAngle"), currentAngle);
	glUniform1i(glGetUniformLocation(shaders[1]->GetProgram(), "cubeTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[2]);

	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::FillBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);

	DrawSkybox();
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	glEnable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawDirectionalLight() {
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(directionallightShader);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(
		directionallightShader->GetProgram(), "depthTex"), 20);
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(
		directionallightShader->GetProgram(), "normTex"), 21);
	glActiveTexture(GL_TEXTURE21);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform1i(glGetUniformLocation(
		directionallightShader->GetProgram(), "shadowTex"), 22);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, directionalLight->GetShadowTex());

	glUniform3fv(glGetUniformLocation(directionallightShader->GetProgram(),
		"cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform2f(glGetUniformLocation(directionallightShader->GetProgram(),
		"pixelSize"), 1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(
		directionallightShader->GetProgram(), "inverseProjView"),
		1, false, invViewProj.values);

	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	shadowMatrix = directionalLight->GetShadowMatrix();
	UpdateShaderMatrices();
	SetShaderLight(*directionalLight);
	quad->Draw();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	viewMatrix = camera->BuildViewMatrix();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Renderer::DrawPointLights() {
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(pointlightShader);

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

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	viewMatrix = camera->BuildViewMatrix();
	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(
		pointlightShader->GetProgram(), "inverseProjView"),
		1, false, invViewProj.values);

	glUniform1f(glGetUniformLocation(pointlightShader->GetProgram(), "far_plane"), 5000.f);

	UpdateShaderMatrices();
	for (int i = 0; i < POINT_LIGHT_NUM; ++i) {
		PointLight& l = pointLights[i];
		SetShaderLight(l);

		Matrix4 faces[CUBE_FACES];
		for (int i = 0; i < CUBE_FACES; i++) {
			faces[i] = l.GetShadowMatrix(i);
		}

		glActiveTexture(GL_TEXTURE30);
		glBindTexture(GL_TEXTURE_CUBE_MAP, l.GetShadowTexCube());
		glUniform1i(glGetUniformLocation(pointlightShader->GetProgram(), "shadowTex"), 30);
		glUniformMatrix4fv(glGetUniformLocation(pointlightShader->GetProgram(), "shadowMatrix"), 6, false, (float*)& faces);
		sphere->Draw();

	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Renderer::DrawSpotLights() {
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(spotlightShader);

	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(
		spotlightShader->GetProgram(), "depthTex"), 20);
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(
		spotlightShader->GetProgram(), "normTex"), 21);
	glActiveTexture(GL_TEXTURE21);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(spotlightShader->GetProgram(),
		"cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform2f(glGetUniformLocation(spotlightShader->GetProgram(),
		"pixelSize"), 1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(
		spotlightShader->GetProgram(), "inverseProjView"),
		1, false, invViewProj.values);


	for (int i = 0; i < SPOT_LIGHT_NUM; ++i) {

		SpotLight& l = spotLights[i];
		Vector3 initialDir = Vector3(0, 0, 1);
		Vector3 finalDir = l.GetDirection();

		float rotationAngle = acos(Vector3::Dot(initialDir, finalDir));
		Vector3 rotationAxis = Vector3::Cross(initialDir,finalDir);
		modelMatrix = 
			Matrix4::Translation(l.GetPosition()) *
			Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f)) *
			Matrix4::Rotation(90, -rotationAxis);
		shadowMatrix = l.GetShadowMatrix();
		UpdateShaderMatrices();
		SetShaderLight(l);
		glUniform1i(glGetUniformLocation(spotlightShader->GetProgram(), "shadowTex"), 22);
		glActiveTexture(GL_TEXTURE22);
		glBindTexture(GL_TEXTURE_2D, l.GetShadowTex());
		glUniform3fv(glGetUniformLocation(spotlightShader->GetProgram(),"lightDirection"), 1, (float*)& l.GetDirection());
		glUniform1f(glGetUniformLocation(spotlightShader->GetProgram(),"lightCutoff"), cos(l.GetRadius() * (PI / 180)));
		cone->Draw();

	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.2f, 0.2f, 0.2f, 1);

	glDepthMask(GL_TRUE);
	modelMatrix.ToIdentity();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Renderer::DeferredLighting() {
	DrawDirectionalLight();
	DrawPointLights();
	DrawSpotLights();
}

void Renderer::CombineBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDepthMask(GL_FALSE);

	BindShader(combineShader);
	modelMatrix.ToIdentity();
	//viewMatrix.ToIdentity();
	//projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseTex"), 23);
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseLight"), 26);
	glActiveTexture(GL_TEXTURE26);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "specularLight"), 27);
	glActiveTexture(GL_TEXTURE27);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	quad->Draw();
	glDepthMask(GL_TRUE);
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::GenerateReflectionBuffer() {
	// Enable clipping distance and set clipping plane
	glEnable(GL_CLIP_DISTANCE0);
	glEnable(GL_CULL_FACE);
	clipPlane = reflectionClipPlane;
	// Move camera down, 2 * the distance to the refraction clipping plane height(water height)
	// and invert the camera pitch
	float distance = 2 * (camera->GetPosition().y - (refractionClipPlane.w - 2.5));
	camera->SetPosition(Vector3(camera->GetPosition().x, camera->GetPosition().y - distance, camera->GetPosition().z));
	camera->SetPitch(-camera->GetPitch());
	viewMatrix = camera->BuildViewMatrix();

	// Draw reflection to GBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// Disable blend to allow skybox to be drawn at alpha 0.0 and still have colour
	glDisable(GL_BLEND);
	DrawSkybox();
	glEnable(GL_BLEND);
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	camera->SetPosition(Vector3(camera->GetPosition().x, camera->GetPosition().y + distance, camera->GetPosition().z));
	camera->SetPitch(-camera->GetPitch());
	viewMatrix = camera->BuildViewMatrix();
	glDisable(GL_CLIP_DISTANCE0);
	glDisable(GL_CULL_FACE);

	// Draw lights for GBuffer
	DeferredLighting();

	// Combine GBuffer with light information, store in reflection FBO
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDepthMask(GL_FALSE);

	BindShader(combineShader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseTex"), 22);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseLight"), 23);
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "specularLight"), 24);
	glActiveTexture(GL_TEXTURE24);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	quad->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::GenerateRefractionBuffer() {
	// Enable clip distance and set clip plane
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glEnable(GL_CLIP_DISTANCE0);
	clipPlane = refractionClipPlane;

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	glDisable(GL_CLIP_DISTANCE0);

	// Draw point lights on GBuffer information
	DeferredLighting();

	// Combine GBuffer with light information, store in refraction FBO
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDepthMask(GL_FALSE);

	BindShader(combineShader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseTex"), 22);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "diffuseLight"), 23);
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(
		combineShader->GetProgram(), "specularLight"), 24);
	glActiveTexture(GL_TEXTURE24);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	quad->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawShadowScene() {
	DrawDirectionalLightShadow();
	DrawPointLightsShadow();
	DrawSpotLightsShadow();
}

void Renderer::DrawDirectionalLightShadow() {
	glBindFramebuffer(GL_FRAMEBUFFER, directionalLight->GetShadowFBO());

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		
	BindShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(
		Vector3((heightmapSize / 2) + (-directionalLight->GetPosition() * 300.0f)), Vector3(heightmapSize.x/2, 0, heightmapSize.z / 2));
	projMatrix = Matrix4::Orthographic(1.0f,5000.0f, -2500.0f, 2500.0f, -2500.0f, 2500.0f);
	shadowMatrix = projMatrix * viewMatrix; //used later
	directionalLight->SetShadowMatrix(shadowMatrix);

	for (const auto& i : nodeList) {
		modelMatrix = i->GetWorldTransform() * Matrix4::Scale(i->GetModelScale());
		UpdateShaderMatrices();
		i->Draw(*this);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::DrawPointLightsShadow() {
	for (int i = 0; i < POINT_LIGHT_NUM; ++i) {
		PointLight& l = pointLights[i];
		glBindFramebuffer(GL_FRAMEBUFFER, l.GetShadowFBO());

		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		BindShader(shadowCubeShader);

		// Set up shadow matrices for light
		projMatrix = Matrix4::Perspective(1, 5000.0f, 1, 90);
		l.SetShadowMatrix(0,projMatrix * Matrix4::BuildViewMatrix(l.GetPosition(),
			l.GetPosition() + Vector3(1.0, 0, 0), Vector3(0, -1.0, 0)));
		l.SetShadowMatrix(1, projMatrix * Matrix4::BuildViewMatrix(l.GetPosition(),
			l.GetPosition() + Vector3(-1.0, 0, 0), Vector3(0, -1.0, 0)));
		l.SetShadowMatrix(2, projMatrix * Matrix4::BuildViewMatrix(l.GetPosition(),
			l.GetPosition() + Vector3(0, 1.0, 0), Vector3(0,0,1.0)));
		l.SetShadowMatrix(3, projMatrix * Matrix4::BuildViewMatrix(l.GetPosition(),
			l.GetPosition() + Vector3(0, -1.0, 0), Vector3(0, 0, -1.0)));
		l.SetShadowMatrix(4, projMatrix * Matrix4::BuildViewMatrix(l.GetPosition(),
			l.GetPosition() + Vector3(0, 0, 1.0), Vector3(0, -1.0, 0)));
		l.SetShadowMatrix(5, projMatrix * Matrix4::BuildViewMatrix(l.GetPosition(),
			l.GetPosition() + Vector3(0, 0, -1.0), Vector3(0, -1.0, 0)));

		Matrix4 faces[CUBE_FACES];
		for (int i = 0; i < CUBE_FACES; i++) {
			faces[i] = l.GetShadowMatrix(i);
		}

		for (const auto& i : nodeList) {
			modelMatrix = i->GetWorldTransform() * Matrix4::Scale(i->GetModelScale());
			SetShaderLight(l);
			UpdateShaderMatrices();
			glUniform1f(glGetUniformLocation(shadowCubeShader->GetProgram(), "far_plane"), 5000.f);
			glUniformMatrix4fv(glGetUniformLocation(shadowCubeShader->GetProgram(), "shadowMatrices"), 6, false, (float*)& faces);
			i->Draw(*this);
		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
			(float)width / (float)height, 45.0f);
		viewMatrix = camera->BuildViewMatrix();
	}
}

void Renderer::DrawSpotLightsShadow() {
	for (int i = 0; i < SPOT_LIGHT_NUM; ++i) {
		SpotLight& l = spotLights[i];
		glBindFramebuffer(GL_FRAMEBUFFER, l.GetShadowFBO());

		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		BindShader(shadowShader);
		viewMatrix = Matrix4::BuildViewMatrix(l.GetPosition(), l.GetDirection() + l.GetPosition());
		projMatrix = Matrix4::Perspective(1, 5000.0f, 1, 45.0f);
		shadowMatrix = projMatrix * viewMatrix; //used later
		l.SetShadowMatrix(shadowMatrix);

		for (const auto& i : nodeList) {
			modelMatrix = i->GetWorldTransform() * Matrix4::Scale(i->GetModelScale());
			UpdateShaderMatrices();
			i->Draw(*this);
		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
			(float)width / (float)height, 45.0f);
		viewMatrix = camera->BuildViewMatrix();
	}
}

vector<Vector4> Renderer::getFrustumCornersWorldSpace()
{
	Matrix4 inv = (projMatrix * viewMatrix).Inverse();

	vector<Vector4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				Vector4 pt =
					inv * Vector4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				//frustumCorners.push_back(Vector4(pt.x,pt.y,pt.z, pt.w) / pt.w);
			}
		}
	}

	return frustumCorners;
}

