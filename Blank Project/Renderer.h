#pragma  once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Matrix3.h"

class  Camera;
class  SceneNode;
class  Mesh;
class  Shader;
class Material;
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

	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;

	SceneNode* root;
	Camera* camera;
	Frustum frameFrustum;
	float sceneTime;
	Light* light;

	Shader* processShader;
	Shader* sceneShader;
	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	Mesh* quad;
	vector<Material*> materials;
	vector<Shader*> shaders;
	vector<GLuint> textures;
	Vector3 heightmapSize;

	Matrix3 normalMatrix;
};