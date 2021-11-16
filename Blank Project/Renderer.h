#pragma  once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
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

	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;

	SceneNode* root;
	Camera* camera;
	Frustum frameFrustum;

	Mesh* quad;
	Mesh* cube;
	Shader* shader;
	Shader* shader2;
	GLuint        texture;
};