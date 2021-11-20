#include "WaveMaterial.h"

WaveMaterial::WaveMaterial() : Material() {
	speed = 40.0f;
	waveLength = 400.0f;
	amplitude = 10.0f;
}

WaveMaterial::~WaveMaterial() {

}

void WaveMaterial::PassShaderUniforms() {
	Material::PassShaderUniforms();
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "speed"), speed);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "waveLength"), waveLength);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "amplitude"), amplitude);

}