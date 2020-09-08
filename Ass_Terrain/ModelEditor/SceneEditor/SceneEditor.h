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
	�Ʒ��� ��� ���� Hiarachy�� ��� �Ѵٸ� ? 
	vector<Shader*> shaderLists;
	vector<ModelAnimMultiBone*> modelMultiBoneLists;
	vector<ModelAnimator*> modelAnimatorLists;
	vector<Terrain*> terrainLists;
	vector�� ���� �ϰ� hiarachy �� unordered_map<int,String,vector<int>>
	Ŭ���� -> unordered_map ���� key�� value �� ã�� value �� ���� �� ���� 
	�ٸ��� inspector �� �����.
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
	//Sky�� �ϳ��Ǿ��� �ϳ��� �����Ѵ�.
	Sky* sky = NULL;
	Shader* shader_57;
private:
	//Terrain�� �Ѱ��� �ҷ� �� �� �ְ� �Ѵ�.
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