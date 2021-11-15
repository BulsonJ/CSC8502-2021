#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	camera = new Camera(0.0f, 0.0f, (Vector3(0, 100, 750.0f)));
	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	heightMapTexture = SOIL_load_OGL_texture(
		TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	heightMapBumpmap = SOIL_load_OGL_texture( //Add this line!
		TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!heightMapTexture || !heightMapBumpmap) {
		return;
	}

	heightMapShader = new Shader("BumpVertex.glsl", "BumpFragment.glsl");

	if(!heightMapShader->LoadSuccess()) {
		return;
	}

	SetTextureRepeating(heightMapTexture, true);
	SetTextureRepeating(heightMapBumpmap, true); 

	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f,
		heightmapSize * Vector3(0.5f, 5.0f, 0.5f));

	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f),
		Vector4(1, 1, 1, 1), heightmapSize.x * 0.5f);

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	init = true;
}
Renderer::~Renderer(void)	{
	delete camera;
	delete heightMap;
	delete heightMapShader;
	delete light;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(heightMapShader);

	glUniform1i(glGetUniformLocation(
		heightMapShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightMapTexture);

	glUniform1i(glGetUniformLocation(
		heightMapShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightMapBumpmap);

	glUniform3fv(glGetUniformLocation(heightMapShader->GetProgram(),
		"cameraPos"), 1, (float*)& camera->GetPosition());

	UpdateShaderMatrices();
	SetShaderLight(*light);
	heightMap->Draw();
}

