#pragma once
#include "Systems/IExecute.h"

class BloomDemo : public IExecute
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
	void Mesh();
	void Airplane();
	void Kachujin();
	
	void Pass(UINT mesh, UINT model, UINT anim);

	void AddPointLights();
	void AddSpotLights();

	void AddBillboard();

private:
	void SetBlur();

	void GetBlurParameter(vector<float>& weights, vector<Vector2>& offsets, float x, float y);
	float GetGaussianFuntion(float val);

private:
	bool bOrigin = true;
	UINT targetIndex = 5;

	float threshold = 0.05f;

	UINT blurCount = 15;

	vector<float> weightX;
	vector<Vector2> offsetX;
	
	vector<float> weightY;
	vector<Vector2> offsetY;

private:
	Shader* shader;
	Shadow* shadow;

	RenderTarget* renderTarget[5];
	DepthStencil* depthStencil;
	Viewport* viewPort;
	Render2D* render2D;
	PostEffect* postEffect;

	Shader* billShader;
	Billboard* billboard;

	Sky* sky;
	Snow* snow;

	Material* floor;
	Material* stone;
	Material* brick;
	Material* wall;

	MeshRender* sphere;
	MeshRender* cylinder;
	MeshRender* cube;
	MeshRender* grid;

	ModelRender* airplane = NULL;

	Model* weapon;
	ModelAnimator* kachujin = NULL;

	struct ColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	} colliders[4];


	vector<MeshRender *> meshes;
	vector<ModelRender *> models;
	vector<ModelAnimator *> animators;
};
