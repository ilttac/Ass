#include "stdafx.h"
#include "TerrainLineDemo.h"
#include "Environment/Terrain.h"
#include "Viewer/Freedom.h"

void TerrainLineDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"22_TerrainLine.fx");
	
	terrain = new Terrain(shader, L"Terrain/Gray256.png");
	terrain->BaseMap(L"Terrain/Dirt3.png");
	
}

void TerrainLineDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(terrain);
}

void TerrainLineDemo::Update()
{

	terrain->Update();
}

void TerrainLineDemo::Render()
{

	terrain->Render();


}


