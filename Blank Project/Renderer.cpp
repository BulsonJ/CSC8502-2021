#include "Renderer.h"
#include "../nclgl/CubeRobot.h"
#include "../nclgl/Camera.h"
#include  <algorithm>                //For  std::sort ...
Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new  Camera(0.0f, 0.0f, (Vector3(0, 100, 750.0f)));
	quad = Mesh::GenerateQuad();
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");
	shader = new  Shader("SceneVertex.glsl", "SceneFragment.glsl");
	shader2 = new  Shader("SceneVertex.glsl", "SceneFragment2.glsl");
	texture = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	if (!shader->LoadSuccess() || !texture) {
		return;
	}

	root = new  SceneNode();
	for (int i = 0; i < 5; ++i) {
		SceneNode* s = new  SceneNode();
		s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
		s->SetTransform(Matrix4::Translation(
			Vector3(0, 100.0f, -300.0f + 100.0f + 100 * i)));
		s->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
		s->SetBoundingRadius(100.0f);
		s->SetMesh(quad);
		s->SetTexture(texture);
		s->SetShader(shader);
		root->AddChild(s);
	}
	CubeRobot* cubeRobot = new CubeRobot(cube);
	cubeRobot->SetShaderOverall(cubeRobot, shader2);
	root->AddChild(cubeRobot);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	init = true;
}

Renderer::~Renderer(void) {
	delete  root;
	delete  quad;
	delete  camera;
	delete  cube;
	delete  shader;
	glDeleteTextures(1, &texture);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	root->Update(dt);
}

void   Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3  dir = from->GetWorldTransform().GetPositionVector() -
			camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));
		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}
	for (vector <SceneNode*>::const_iterator i =
		from->GetChildIteratorStart();
		i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(),    //note  the r!
		transparentNodeList.rend(),      //note  the r!
		SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),
		nodeList.end(),
		SceneNode::CompareByCameraDistance);
}

void   Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void   Renderer::DrawNode(SceneNode* n) {

	if (n->GetMesh()) {
		if (n->GetShader()) {
			Shader* shader = (Shader*)(n->GetShader());

			BindShader(shader);
			UpdateShaderMatrices();
			glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);


			Matrix4  model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);
			glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "viewMatrix"), 1, false, viewMatrix.values);
			glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "projMatrix"), 1, false, projMatrix.values);
			glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());

			// Get texture of scene node, if scene node has texture it will be bound and uniform set to 1,
			// otherwise set to 0
			GLuint texture = n->GetTexture();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), texture);
			n->Draw(*this);
		}
	}
}

void  Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawNodes();
	ClearNodeLists();
}

void   Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}