#pragma once

#include "Vector4.h"
#include "Vector3.h"
#include <glad\glad.h>
#include "Matrix4.h"

class Light {
public:
	Light() {} // Default constructor , we’ll be needing this later!
	Light(const Vector3 & position, const Vector4 & colour, float radius, bool useShadows=false) {
		this->position = position;
		this->colour = colour;
		this->radius = radius;
	}

	~Light(void);

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3 & val) { position = val; }
	
	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4 & val) { colour = val; }

	GLuint GetShadowFBO() const { return shadowFBO; }
	void CreateShadowFBO();

	GLuint GetShadowTex() const { return shadowTex; }

	Matrix4 GetShadowMatrix() const { return shadowMatrix; }
	void SetShadowMatrix(const Matrix4 matrix) { shadowMatrix = matrix; }

protected:
	Vector3 position;
	float radius;
	Vector4 colour;

	Matrix4 shadowMatrix;

	GLuint shadowFBO;
	GLuint shadowTex;
};
