#pragma once
#pragma once
#include "Material.h"
class TerrainMaterial : public Material
{
public:
	TerrainMaterial();
	~TerrainMaterial();

	virtual void PassShaderUniforms();

	void      SetSandTex(GLuint  tex) { sandTex = tex; }
	GLuint    GetSandTex()  const { return  sandTex; }

	void      SetSandNormal(GLuint  tex) { sandNormal = tex; }
	GLuint    GetSandNormal()  const { return  sandNormal; }

	void      SetGrassTex(GLuint  tex) { grassTex = tex; }
	GLuint    GetGrassTex()  const { return  grassTex; }

	void      SetGrassNormal(GLuint  tex) { grassNormal = tex; }
	GLuint    GetGrassNormal()  const { return  grassNormal; }

	void      SetRockTex(GLuint  tex) { rockTex = tex; }
	GLuint    GetRockTex()  const { return  rockTex; }

	void      SetRockNormal(GLuint  tex) { rockNormal = tex; }
	GLuint    GetRockNormal()  const { return  rockNormal; }

protected:

	GLuint sandTex;
	GLuint sandNormal;

	GLuint grassTex;
	GLuint grassNormal;

	GLuint rockTex;
	GLuint rockNormal;

};



