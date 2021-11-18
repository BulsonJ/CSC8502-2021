#include "Material.h"

Material::Material() {
	this->shader = NULL;
	texture = 0;
	bumpTexture = 0;
	cubeMap = 0;
}

Material::~Material() {

}

void Material::PassShaderUniforms() {
	if (texture != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	if (bumpTexture != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bumpTexture);
	}

	if (cubeMap != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "cubeTex"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
	}
}