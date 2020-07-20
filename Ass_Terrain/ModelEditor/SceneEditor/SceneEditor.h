#pragma once
#include "Systems/IExecute.h"

class SceneEditor : public IExecute
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
	void MainMenu();
	void OpenHeightMap(wstring file);
	void OpenTrnFile(wstring file);

	void Inspector();
	void TerrainInspector();
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
	//Terrain은 한개만 불러 올 수 있게 한다.
	Terrain* terrain = NULL;
	wstring heightMapName;
	wstring openTerrainFile;
	wstring saveTerrainFile;
private:
	Shader* shader;
	Shader* skyShader;
	Shadow* shadow;

	Sky* sky;
};