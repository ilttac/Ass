#include "stdafx.h"
#include "TerrainSplattingDemo.h"
#include "Environment/Terrain.h"
#include "Viewer/Freedom.h"

void TerrainSplattingDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"23_TerrainSpatting.fx");
	
	terrain = new Terrain(shader, L"Terrain/Gray256.png");
	terrain->BaseMap(L"Terrain/Rock (Basic).jpg");
	terrain->LayerMap(L"Terrain/Path (Rocky).jpg", L"Terrain/Splatting.png");
}

void TerrainSplattingDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(terrain);
}

void TerrainSplattingDemo::Update()
{

	terrain->Update();
}

void TerrainSplattingDemo::Render()
{

	terrain->Render();


}


