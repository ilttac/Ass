#pragma once

#include "Systems/IExecute.h"

class ModelEditor : public IExecute
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


	void MainMenu();
	void Project();
	void Hiarachy();
	void Inspector();
	void WriteMeshFile(wstring file);
	void WriteMaterialFile(wstring file);
	void OpenFile(wstring file);
	void OpenFbxFile(wstring file);
	void OpenMeshFile(wstring file);


private:
	Shader* shader;
	Shader* modelShader;
	Shadow* shadow;

	ParticleSystem* particleSystem;
	ModelRender* modelAnimator = NULL;
	float windowWidth = 500;

	bool bLoop = false;
	UINT maxParticle = 0;

	vector<wstring> modelAnimList;
	vector<wstring> particleList;
	vector<wstring> textureLists;
	vector<wstring> boneLists;
	vector<wstring> meshLists;
	vector<wstring> clipLists;

	wstring openFile = L"";
	wstring saveFile = L"";
	Sky* sky;

	Material* floor;
	Material* stone;

	MeshRender* sphere;
	MeshRender* grid;

	vector<MeshRender*> meshes;
	//-
private:
	wstring file;

	Assimp::Importer* importer;
	const aiScene* scene;

	vector<struct asMaterial*> materials;

	vector<struct asBone*> bones;
	vector<struct asMesh*> asMeshes;
};
