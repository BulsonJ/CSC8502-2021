#include "WaveMaterial.h"

WaveMaterial::WaveMaterial() : Material() {
	speed = 40.0f;
	waveLength = 400.0f;
	amplitude = 10.0f;
	refractionTex = 0;
	reflectionTex = 0;
}

WaveMaterial::~WaveMaterial() {

}

void WaveMaterial::PassShaderUniforms() {
	Material::PassShaderUniforms();
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "speed"), speed);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "waveLength"), waveLength);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "amplitude"), amplitude);

	if (reflectionTex != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "reflectionTex"), 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, reflectionTex);
	}

	if (refractionTex != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "refractionTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, refractionTex);
	}

}