#pragma once
#include "Systems/IExecute.h"

class SceneEditor : public IExecute
{
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
	void MainMenu();
	void OpenHeightMap(wstring file);
	void OpenTrnFile(wstring file);

	void Inspector();
	void TerrainInspector();
	void SkyInspector();

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
private:
	Shader* shader_53; //gBuffer
	Shadow* shadow;
	GBuffer* gBuffer;
};