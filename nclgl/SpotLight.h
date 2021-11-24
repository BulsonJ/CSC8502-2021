#pragma once
#include "Light.h"
#include "Mesh.h"

class SpotLight : public Light
{
public:
	SpotLight() {};
	SpotLight(const Vector3& position, const Vector4& colour, float radius);
	~SpotLight(void);

	GLuint GetShadowFBO() const { return shadowFBO; }
	void CreateShadowFBO();

	GLuint GetShadowTex() const { return shadowTex; }

	Matrix4 GetShadowMatrix() const { return shadowMatrix; }
	void SetShadowMatrix(const Matrix4 matrix) { shadowMatrix = matrix; }

	Vector3 GetTargetPosition() const { return targetPos; }
	void SetTargetPosition(const Vector3& val) { targetPos = val; }

protected:
	Vector3 targetPos;

	Matrix4 shadowMatrix;

	GLuint shadowFBO;
	GLuint shadowTex;
};

