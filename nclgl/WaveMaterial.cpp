#include "WaveMaterial.h"

WaveMaterial::WaveMaterial() : Material() {
	speed = 10.0f;
	waveLength = 400.0f;
	amplitude = 10.0f;
	passDepthTexture = false;
}

WaveMaterial::~WaveMaterial() {

}

void WaveMaterial::PassShaderUniforms() {
	Material::PassShaderUniforms();

	/*if (passDepthTexture) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 5);
		glActiveTexture(GL_TEXTURE5);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	}*/

	glUniform1f(glGetUniformLocation(shader->GetProgram(), "speed"), speed);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "waveLength"), waveLength);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "amplitude"), amplitude);

}