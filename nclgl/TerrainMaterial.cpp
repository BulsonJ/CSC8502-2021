#include "TerrainMaterial.h"

TerrainMaterial::TerrainMaterial() : Material() {
	sandTex = 0;
	sandNormal = 0;
	grassTex = 0;
	grassNormal = 0;
	rockTex = 0;
	rockNormal = 0;
}

TerrainMaterial::~TerrainMaterial() {

}

void TerrainMaterial::PassShaderUniforms() {
	Material::PassShaderUniforms();


	if (sandTex != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "sandTex"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, sandTex);
	}

	if (sandNormal != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "sandNormal"), 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, sandNormal);
	}

	if (grassTex != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "grassTex"), 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, grassTex);
	}

	if (grassNormal != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "grassNormal"), 8);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, grassNormal);
	}

	if (rockTex != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "rockTex"), 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, rockTex);
	}

	if (rockNormal != 0) {
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "rockNormal"), 10);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, rockNormal);
	}

}