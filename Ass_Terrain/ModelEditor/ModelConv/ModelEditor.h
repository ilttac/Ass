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
	void Weapon();

private:
	//ImGui
	void MainMenu();
	void Project();
	void Hiarachy();
	void Inspector();
	void Gizmo();
	void Animation();

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
	vector<vector<string>> clipLists;

	map<int, Collider*> colliders;
	map<int, Transform*> colliderTransforms;


	wstring openFile = L"";
	wstring openPngFile = L"";
	wstring saveFile = L"";

	Texture* playeButton;
	Texture* pauseButton;
	Texture* stopButton;

	Sky* sky;

	Material* floor;
	Material* stone;

	MeshRender* sphere;
	MeshRender* grid;
	

	vector<MeshRender*> meshes;
	//-

private:
	wstring file;
	vector<Texture*> textures;
	vector<ModelBone*> modelBones;

	//
private:
	ModelRender* weapon;
	//

//Project,Hiarachy
private:
	vector<string> projectMeshNames;
	vector<string> projectPngFileNames;
	vector<string> hiarachyName;
	vector<ModelAnimMultiBone*> modelLists;
	vector<string> clipNames;

	int currentModelID = -1;
	int currentBoneIndex = -1;
private:
	int lastUsing = 0;

private:
	enum EditorState
	{
		MESH_EDITOR_STATE =0,
		BONE_EDITOR_STATE,
		ANIM_EDITOR_STATE
	};

	EditorState editorState = MESH_EDITOR_STATE;
	enum { eClipNameMaxNum = 100 };

	bool tempDebugvalue = false;
private:
	bool bCount = false;
};
