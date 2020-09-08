#pragma once
#include "Systems/IExecute.h"

class SceneEditor : public IExecute
{
	friend class LevelSaveLoad;
	friend class Terrain;
public:
	virtual void Initialize() override;
	virtual void Ready() override {};
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override ;
	virtual void Render() override;
	virtual void PostRender() override ;
	virtual void ResizeScreen() override {};
private:
	void OpenHeightMap(wstring file);
	void OpenTrnFile(wstring file);
	void OpenTerrainMapFile(wstring file);
	void StaticMeshLoad(wstring file);
	void BillboardSet();

	//level
	void SaveLevelFile(wstring file);
	void OpenLevelFile(wstring file);

	void MainMenu();
	void Hiarachy();
	void ViewModel();
	void Inspector();
	void TerrainInspector();
	void SkyInspector();
	void BillboardInspector();
	void GuizmoUpdate(int clickedNodeNum);
	void DragAndDropTreeNode(const char* label);

private:
	//Object Lists
	/*
	아래를 모두 같은 Hiarachy에 등록 한다면 ? 
	vector<Shader*> shaderLists;
	vector<ModelAnimMultiBone*> modelMultiBoneLists;
	vector<ModelAnimator*> modelAnimatorLists;
	vector<Terrain*> terrainLists;
	vector로 관리 하고 hiarachy 는 unordered_map<int,String,vector<int>>
	클릭시 -> unordered_map 에서 key로 value 를 찾고 value 의 유형 에 따라 
	다르게 inspector 를 만든다.
	*/
private:
	//Hiarachy
	wstring openModelFile = L"";
	
	//ModelRenerHiarachy
	vector<string> hiarachyName;
	vector<Matrix*> hiarachyMatrix;
	//
	
	int node_clicked = -1;
	
private:
	//ModelLists
	Shader* modelShader = NULL;
	vector<string> modelNames;
	vector<ModelRender*> modelLists;
private:
	//ModelAnimLists
	vector<string> modelAnimNames;

private:
	//Gizmo
	bool isPerspective = true;
	float fov = 27.f;
	float viewWidth = 10.f; // for orthographic
	float camYAngle = 165.f / 180.f * 3.14159f;
	float camXAngle = 32.f / 180.f * 3.14159f;
	float camDistance = 8.f;
	int gizmoCount = 1;
	int lastUsing = 0;
	
private:
	//Sky은 하나의씬에 하나만 존재한다.
	Sky* sky = NULL;
	Shader* shader_57;
private:
	//Terrain은 한개만 불러 올 수 있게 한다.
	Terrain* terrain = NULL;
	wstring heightMapName;
	wstring openTerrainFile;
	wstring saveTerrainFile;
	Shader* terrainShader;
	wstring openTerrainMapFile;

	//level
	wstring saveLevelFile;
	wstring loadLevelFile;

	enum class ETerrainMapState
	{
		eNone,
		eBaseMap,
		eLayerMap,
		eAlphaMap
	};

	ETerrainMapState terrainMapState = ETerrainMapState::eNone;

private:
	//Billboard
	Billboard* billBoard = NULL;
	vector<wstring> billBoardNames;
	vector<Texture*> textureList;
	Shader* shader_56; //billboard.fxo
	int billBoardCurID = -1;
private:
	Shader* shader_53; //gBuffer
	Shadow* shadow;
	GBuffer* gBuffer;

private:
	enum class EInsPectorState
	{
		eNone = 0,
		eTerrainEdit ,
		eSkyEdit ,
		eModelEdit,
		eFogEdit,
		eBillboardEdit
	};
	EInsPectorState InspectorState = EInsPectorState::eNone;
};