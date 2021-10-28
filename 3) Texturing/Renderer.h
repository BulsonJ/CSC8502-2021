#pragma  once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"

class  Renderer : public  OGLRenderer {
public:
	Renderer(Window &parent );
	virtual ~Renderer(void);
	virtual  void  RenderScene ();
	void   UpdateTextureMatrix(float  rotation );
	void   ToggleRepeating ();
	void   ToggleFiltering ();

	virtual  void  UpdateScene(float dt);

protected:
	Shader * shader;
	Mesh * triangle;
	GLuint    texture;
	GLuint    texture2;
	bool      filtering;
	bool      repeating; 
	Camera* camera;
};

