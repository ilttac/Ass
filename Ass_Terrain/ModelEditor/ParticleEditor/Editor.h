#pragma once

#include "Systems/IExecute.h"

class Editor : public IExecute
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
	void Mesh();
	void Pass(UINT meshPass);

	void UpdateParticleList();
	void UpdateTextureList();

	void OnGUI();
	void OnGUI_LIst();
	void OnGUI_Settings();
	void OnGuI_Write();
	void WrtieFile(wstring file);

private:
	Shader * shader;
	Shadow* shadow;

	ParticleSystem* particleSystem;

	float windowWidth = 500;

	bool bLoop = false;
	UINT maxParticle = 0;

	vector<wstring> particleList;
	vector<wstring> textureList;

	wstring file = L"";

	Sky* sky;

	Material* floor;
	Material* stone;

	MeshRender* sphere;
	MeshRender* grid;

	vector<MeshRender*> meshes;
};
