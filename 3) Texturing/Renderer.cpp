#include "Renderer.h"
Renderer :: Renderer(Window &parent) : OGLRenderer(parent) {
	triangle = Mesh:: GenerateTriangle ();
	camera = new Camera();

	texture   = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga",
		SOIL_LOAD_AUTO ,SOIL_CREATE_NEW_ID , SOIL_FLAG_MIPMAPS);
	texture2 = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if(!texture || !texture2) {
		return;
	}

	shader = new  Shader("TexturedVertex.glsl", "texturedfragment.glsl");
	if (!shader->LoadSuccess()) {
		return;
	}
	filtering = true;
	repeating = false;
	init = true;
}

Renderer ::~Renderer(void) {
	delete  triangle;
	delete  shader;
	glDeleteTextures(1, &texture);
	glDeleteTextures(1, &texture2);
}

void  Renderer::RenderScene() {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(),
	"diffuseTex"), 0);                 //this  last  parameter
	glActiveTexture(GL_TEXTURE0);    // should  match  this  number!
	glBindTexture(GL_TEXTURE_2D , texture );

	glUniform1i(glGetUniformLocation(shader->GetProgram(),
		"diffuseTex2"), 1);                 //this  last  parameter
	glActiveTexture(GL_TEXTURE1);    // should  match  this  number!
	glBindTexture(GL_TEXTURE_2D, texture2);

	triangle ->Draw ();

	// set colours of vertices
	glBindBuffer(GL_ARRAY_BUFFER, triangle->GetBufferObject(1));
	Vector4* ptr = (Vector4*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	ptr[0] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	ptr[1] = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	ptr[2] = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void  Renderer::UpdateTextureMatrix(float  value) {
	Matrix4  push = Matrix4::Translation(Vector3(-0.5f, -0.5f, 0));
	Matrix4  pop = Matrix4::Translation(Vector3(0.5f, 0.5f, 0));
	Matrix4  rotation = Matrix4::Rotation(value, Vector3(0, 0, 1));
	textureMatrix = pop * rotation * push;
}

void   Renderer::ToggleRepeating() {
	repeating = !repeating;
	SetTextureRepeating(texture, repeating);
}

void   Renderer::ToggleFiltering() {
	filtering = !filtering;
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		filtering ? GL_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		filtering ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		filtering ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_LINEAR_MIPMAP_NEAREST,
		filtering ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}