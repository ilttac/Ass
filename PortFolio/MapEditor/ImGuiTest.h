#pragma once
#include "Systems/IExecute.h"

class ImGuiTest : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override {}

private:
	void Pass(UINT mesh, UINT model, UINT anim);
	void Mesh();

private:
	Shader* shader;

	Shadow* shadow;

	Sky* sky;

	MeshRender* sphere;
	MeshRender* grid;

	Material* floor;


	vector<MeshRender*> meshes;
	vector<ModelRender*> models;
	vector<ModelAnimator*> animators;
};
