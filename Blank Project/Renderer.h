#pragma  once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Matrix3.h"

class  Camera;
class  SceneNode;
class  Mesh;
class  Shader;
class Material;
class SpotLight;
class DirectionalLight;
class  Renderer : public  OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void  UpdateScene(float  msec) override;
	void  RenderScene()  override;
protected:
	void          BuildNodeLists(SceneNode* from);
	void          SortNodeLists();
	void          ClearNodeLists();
	void          DrawNodes();
	void          DrawNode(SceneNode* n);
	//virtual void UpdateShaderMatrices();
	
	void DrawSkybox();
	void DrawDepth();

	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;

	SceneNode* root;
	Camera* camera;
	Frustum frameFrustum;
	float sceneTime;

	DirectionalLight* directionalLight;
	Light* pointLights;
	SpotLight* spotLights;

	Shader* directionallightShader;
	Shader* pointlightShader; // Shader to calculate lighting
	Shader* spotlightShader;
	Shader* combineShader; // shader to stick it all together

	GLuint refractionFBO;
	GLuint refractionBufferTex;
	Vector4 refractionClipPlane;

	GLuint reflectionFBO;
	GLuint reflectionBufferTex;
	Vector4 reflectionClipPlane;

	Mesh* quad;
	Mesh* sphere; // Light volume
	Mesh* cone;

	vector<Material*> materials;
	vector<Shader*> shaders;
	vector<GLuint> textures;
	Vector3 heightmapSize;

	Matrix3 normalMatrix;
	Vector4 clipPlane;

	void FillBuffers(); //G-Buffer Fill Render Pass
	void DrawDirectionalLight();
	void DrawPointLights(); // Lighting Render Pass
	void DrawSpotLights();

	void DrawDirectionalLightShadow();
	void DrawPointLightsShadow();
	void DrawSpotLightsShadow();

	void CombineBuffers(); // Combination Render Pass
	void DeferredLighting();

	void GenerateScreenTexture(GLuint& into, bool depth = false);


	GLuint bufferFBO; //FBO for our G-Buffer pass
	GLuint bufferColourTex; // Albedo goes here
	GLuint bufferNormalTex; // Normals go here
	GLuint bufferDepthTex; // Depth goes here

	GLuint pointLightFBO; //FBO for our lighting pass
	GLuint lightDiffuseTex; // Store diffuse lighting
	GLuint lightSpecularTex; // Store specular lighting


	void SetShaderLights(Shader* shader);
	void GenerateRefractionBuffer();
	void GenerateReflectionBuffer();

	void DrawShadowScene();

	Shader* sceneShader;
	Shader* shadowShader;

};