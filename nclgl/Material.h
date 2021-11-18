#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include < vector >
class Material
{
public:
	Material();
	~Material();

	void      SetShader(Shader*  shad) { shader = shad; }
	Shader*    GetShader()  const { return  shader; }

	void      SetTexture(GLuint  tex) { texture = tex; }
	GLuint    GetTexture()  const { return  texture; }

	void      SetBump(GLuint  bump) { bumpTexture = bump; }
	GLuint    GetBump()  const { return  bumpTexture; }

	void      SetCubeMap(GLuint  cub) { cubeMap = cub; }
	GLuint    GetCubeMap()  const { return  cubeMap; }

	virtual void PassShaderUniforms();
protected:

	Shader* shader;
	GLuint texture;
	GLuint bumpTexture;
	GLuint cubeMap;

	bool useDepth;
};

