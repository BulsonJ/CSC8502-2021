#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/Frustum.h"

class Camera;
class Light; // Predeclare our new class type ...
class Shader;
class HeightMap;

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;

	 void DrawWater();
protected:
	Camera* camera;
	Frustum frameFrustum;
	float sceneTime;

	Light* light;
	Shader* heightMapShader;

	HeightMap* heightMap;
	GLuint heightMapTexture;
	GLuint heightMapBumpmap;

};
