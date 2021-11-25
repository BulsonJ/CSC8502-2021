#include "PointLight.h"
#define SHADOWSIZE 2048
PointLight::PointLight(const Vector3 & position, const Vector4 & colour, float radius) : Light(position, colour, radius) {
	for (int i = 0; i < CUBE_FACES; i++) {
		shadowMatrix[i] = Matrix4();
	}
}

PointLight::~PointLight() {
	glDeleteTextures(1, &shadowTexCube);
	glDeleteFramebuffers(1, &shadowFBO);
}


void PointLight::CreateShadowFBO() {
	// Generate shadow texture and buffer
	glGenTextures(1, &shadowTexCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}


	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexCube, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
