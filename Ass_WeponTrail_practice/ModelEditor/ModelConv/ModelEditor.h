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
private:
	//ImGui
	void MainMenu();
	void Project();
	void Hiarachy();
	void Inspector();
	void Gizmo();
private:
	void WriteMeshFile(wstring file);
	void WriteMaterialFile(wstring file);
	void OpenFile(wstring file);
	void OpenFbxFile(wstring file);
	void OpenMeshFile(wstring file);
	void OpenPngFile(wstring file);
private:
	void BoneView();
	void BoneSphereUpdate();
	void DragAndDropTreeNode(const char* label);
	int DragAndDropReceiver(const char* name);

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
	wstring openPngFile = L"";
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

	//vector<struct asBone*> bones;
	vector<struct asMesh*> asMeshes;
	vector<ModelBone*> modelBones;
//Project,Hiarachy
private:
	vector<string> projectMeshNames;
	vector<string> projectPngFileNames;
	vector<string> hiarachyName;
	vector<ModelRender*> modelLists;
	int currentModelID = -1;
private:
	int lastUsing = 0;
	float objectMatrix[4][16] = {
	  { 1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f },

	  { 1.f, 0.f, 0.f, 0.f,
	  0.f, 1.f, 0.f, 0.f,
	  0.f, 0.f, 1.f, 0.f,
	  2.f, 0.f, 0.f, 1.f },

	  { 1.f, 0.f, 0.f, 0.f,
	  0.f, 1.f, 0.f, 0.f,
	  0.f, 0.f, 1.f, 0.f,
	  2.f, 0.f, 2.f, 1.f },

	  { 1.f, 0.f, 0.f, 0.f,
	  0.f, 1.f, 0.f, 0.f,
	  0.f, 0.f, 1.f, 0.f,
	  0.f, 0.f, 2.f, 1.f }
	};

	float cameraView[16] =
	{ 1.f, 0.f, 0.f, 0.f,
	  0.f, 1.f, 0.f, 0.f,
	  0.f, 0.f, 1.f, 0.f,
	  0.f, 0.f, 0.f, 1.f };

	float cameraProjection[16];
	// Camera projection
	bool isPerspective = true;
	float fov = 27.f;
	float viewWidth = 10.f; // for orthographic
	float camYAngle = 165.f / 180.f * 3.14159f;
	float camXAngle = 32.f / 180.f * 3.14159f;
	float camDistance = 8.f;
	int gizmoCount = 1;

	bool firstFrame = true;
	////
	class Mesh* mesh1;

private:
	enum EditorState
	{
		MESH_EDITOR_STATE =0,
		BONE_EDITOR_STATE
	};
	EditorState editorState = MESH_EDITOR_STATE;
	Matrix matrixIdentity;
	UINT transformNum;
private:
	bool bCount = false;
};
