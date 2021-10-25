#pragma once
#include "C:\Users\b8025171\source\repos\Graphics\CSC8502 2021\nclgl\OGLRenderer.h"
class  Renderer : public  OGLRenderer { 
public:
	Renderer(Window & parent);
	virtual ~Renderer(void);
virtual  void  RenderScene(); 
protected:
	Mesh * triangle; 
	Shader * basicShader; 
};
