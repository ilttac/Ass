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
	//TerrainLod
	TerrainLod::InitialDesc terrainInitialDesc;
	TerrainLod* terrainLod;
	
private:
	Shader* shader;
	Shader* skyShader;
	Shadow* shadow;


	Sky* sky;

};