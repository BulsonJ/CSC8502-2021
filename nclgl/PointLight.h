#pragma once
#include "Light.h"
#include "Mesh.h"
const int CUBE_FACES = 6;
class PointLight : public Light
{
public:
	PointLight() {};
	PointLight(const Vector3& position, const Vector4& colour, float radius);
	~PointLight(void);

	GLuint GetShadowFBO() const { return shadowFBO; }
	void CreateShadowFBO();

	GLuint GetShadowTexCube() const { return shadowTexCube; }

	Matrix4 GetShadowMatrix() { return *shadowMatrix; }
	Matrix4 GetShadowMatrix(int face) const { return shadowMatrix[face]; }
	void SetShadowMatrix(const int face, const Matrix4 matrix) { shadowMatrix[face] = matrix; }


protected:

	Matrix4 shadowMatrix[CUBE_FACES];

	GLuint shadowFBO;
	GLuint shadowTexCube;
};


