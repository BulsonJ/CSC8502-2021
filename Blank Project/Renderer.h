#pragma  once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Matrix3.h"

class MeshMaterial;
class  Camera;
class  SceneNode;
class  Mesh;
class  Shader;
class Material;
class SpotLight;
class DirectionalLight;
class PointLight;
class  Renderer : public  OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void  UpdateScene(float  msec) override;
	void  RenderScene()  override;

	bool lockedCamera;
protected:
	void          BuildNodeLists(SceneNode* from);
	void          SortNodeLists();
	void          ClearNodeLists();
	void          DrawNode(SceneNode* n);

	void GenerateScreenTexture(GLuint& into, bool depth = false);
	void SetShaderLights(Shader* shader);

	void FillBuffers(); //G-Buffer Fill Render Pass
	void DrawDirectionalLight();
	void DrawPointLights(); // Lighting Render Pass
	void DrawSpotLights();

	void DrawDirectionalLightShadow();
	void DrawPointLightsShadow();
	void DrawSpotLightsShadow();

	void CombineBuffers(); // Combination Render Pass
	void DeferredLighting();

	void GenerateRefractionBuffer();
	void GenerateReflectionBuffer();

	void DrawSkybox();
	void DrawShadowScene();
	


	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;

	SceneNode* root;
	Camera* camera;
	Frustum frameFrustum;
	float sceneTime;
	float dayNightTime;
	float currentAngle;

	DirectionalLight* directionalLight;
	PointLight* pointLights;
	SpotLight* spotLights;

	Shader* directionallightShader;
	Shader* pointlightShader; // Shader to calculate lighting
	Shader* spotlightShader;
	Shader* combineShader; // shader to stick it all together

	Shader* sceneShader;
	Shader* shadowShader;
	Shader* shadowCubeShader;
	Shader* skyboxShader;

	GLuint refractionFBO;
	GLuint refractionBufferTex;
	Vector4 refractionClipPlane;

	GLuint reflectionFBO;
	GLuint reflectionBufferTex;
	Vector4 reflectionClipPlane;

	Mesh* quad;
	Mesh* sphere; // Light volume
	Mesh* cone;
	Mesh* capsule;
	Mesh* palmTree;

	vector<Material*> materials;
	vector<Shader*> shaders;
	vector<GLuint> textures;

	Vector3 heightmapSize;
	Matrix3 normalMatrix;
	Vector4 clipPlane;




	GLuint bufferFBO; //FBO for our G-Buffer pass
	GLuint bufferColourTex; // Albedo goes here
	GLuint bufferNormalTex; // Normals go here
	GLuint bufferDepthTex; // Depth goes here

	GLuint deferredLightFBO; //FBO for our lighting pass
	GLuint lightDiffuseTex; // Store diffuse lighting
	GLuint lightSpecularTex; // Store specular lighting

	vector<Vector3> cameraPoints;
	vector<Vector3> cameraRotations;
	int currentPoint;
	float waitTime;
};