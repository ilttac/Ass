#pragma once

#include "Systems/IExecute.h"

class MrtDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {};
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void CreateViewer();

	void Mesh();
	void Airplane();
	void Kachujin();

	void AddPointLights();
	void AddSpotLights();

	void Pass(UINT mesh, UINT model, UINT anim);


private:
	RenderTarget * renderTarget;
	RenderTarget * mrt[2];
	DepthStencil* depthStencil;
	Viewport* viewport;

	Render2D* renderTarget2D;
	Render2D* left2D;
	Render2D* right2D;

	Shader * shader;
	
	class SkyCube* sky;

	Material* floor;
	Material* stone;
	Material* brick;
	Material* wall;

	MeshRender* sphere;
	MeshRender* cylinder;
	MeshRender* cube;
	MeshRender* grid;

	ModelRender* airPlane = NULL;

	Model* weapon;
	ModelAnimator* kachujin = NULL;

	struct ColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	} collider[4];
	

	vector<MeshRender*> meshes;
	vector<ModelRender*> models;
	vector<ModelAnimator*> animators;

};
