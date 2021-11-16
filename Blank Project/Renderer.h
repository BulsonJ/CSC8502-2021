#pragma  once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"

class  Camera;
class  SceneNode;
class  Mesh;
class  Shader;
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

	void AddHeightMap();

	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;

	SceneNode* root;
	Camera* camera;
	Frustum frameFrustum;
	float sceneTime;
	Light* light;

	Mesh* quad;
	Mesh* cube;
	vector<Shader*> shaders;
	vector<GLuint> textures;
};