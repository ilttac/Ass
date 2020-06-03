#pragma once

#include "Systems/IExecute.h"

class QuadTreeFrustum : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {};
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {};
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void CreateMeshData();
	void GuiSet();

private:
	Material * red;
	Shader* shader;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	PerFrame* perFrame;

	FrustumCamera* frustumCamera;

	Shader* terrainShader;
	vector<Transform*> transforms;
	
	Transform* t;

	Material* floor;
	Shader* gridShader;
	MeshRender* grid;
	
	float fov = 0.25f;
	float zFar = 100.0f;

	float height = 768;
	float width = 1024;

	Fixity* camera;
	Perspective* perspective;
	Frustum* frustum;
	QuadTree* quadTree;

	Shader* modelShader;
	ModelRender* model;

	class Terrain* terrain;
};
