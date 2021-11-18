#include "SceneNode.h"

SceneNode::SceneNode(Mesh* mesh, Vector4 colour) {
	this-> mesh = mesh;
	this-> shader = NULL;
	this-> colour = colour;
	parent = NULL;
	modelScale = Vector3(1, 1, 1);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
	bumpTexture = 0;
	cubeMap = 0;
	useLight = true;
}


SceneNode ::~SceneNode(void) {
	for (unsigned int i = 0; i < children.size(); ++i) {
		delete children[i];
	}
}

void SceneNode::AddChild(SceneNode * s) {
	if (this == s) {
		return;
	}

	children.push_back(s);
	s-> parent = this;
}

void SceneNode::RemoveChild(SceneNode* s) {
	auto it = std::find(children.begin(), children.end(), s);
	if (it != children.end()) {
		delete s;
		children.erase(it);
	}
}

void SceneNode::Draw(const OGLRenderer& r) {
	if (mesh) { mesh->Draw(); }
}

void SceneNode::Update(float dt) {
	if (parent) { // This node has a parent ...
		worldTransform = parent->worldTransform * transform;
	}
	else { // Root node , world transform is local transform !
		worldTransform = transform;
	}
	for (vector < SceneNode* >::iterator i = children.begin();
		i != children.end(); ++i) {
		(*i)->Update(dt);
	}
}

void SceneNode::SetScale(SceneNode* n, int scale) {
	if (n->GetMesh()) {
		n->SetModelScale(n->GetModelScale() * Vector3(scale, scale, scale));
	}

	for (vector < SceneNode* >::const_iterator
		i = n->GetChildIteratorStart();
		i != n->GetChildIteratorEnd(); ++i) {
		SetScale(*i, scale);
	}
}

void SceneNode::SetShaderOverall(SceneNode* n, Shader* shader) {
	if (n->GetMesh()) {
		n->SetShader(shader);
	}

	for (vector < SceneNode* >::const_iterator
		i = n->GetChildIteratorStart();
		i != n->GetChildIteratorEnd(); ++i) {
		SetShaderOverall(*i, shader);
	}
}