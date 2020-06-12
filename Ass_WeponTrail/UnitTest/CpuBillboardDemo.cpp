#include "stdafx.h"
#include "CpuBillboardDemo.h"
#include "Environment/Terrain.h"
#include "Viewer/Freedom.h"
#include "Environment/SkyCube.h"
#include "Environment/Billboard.h"

void CpuBillboardDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"23_TerrainSpatting.fx");
	
	terrain = new Terrain(shader, L"Terrain/Gray256.png");
	terrain->BaseMap(L"Terrain/Dirt3.png");
	terrain->LayerMap(L"Terrain/Grass (Hill).jpg", L"Terrain/Dirt3.png");

	sky = new SkyCube(L"Environment/GrassCube1024.dds");

	bbShader = new Shader(L"27_Billboard.fx");
	for (UINT i = 0; i < 1000; i++)
	{
		bb[i] = new Billboard(bbShader, L"Terrain/Tree.png");

		Vector2 scale = Math::RandomVec2(3.0f, 10.0f);
		bb[i]->GetTransform()->Scale(scale.x, scale.y, 1.0f);

		Vector3 position = Math::RandomVec3(0.0f, 255.0f);
		position.y = terrain->GetHeight(position) + scale.y * 0.5f;
		bb[i]->GetTransform()->Position(position);
	}
}

void CpuBillboardDemo::Destroy()
{
	SafeDelete(sky);
	SafeDelete(shader);
	SafeDelete(terrain);

	for (Billboard* b : bb)
		SafeDelete(b);
}

void CpuBillboardDemo::Update()
{
	sky->Update();
	terrain->Update();

	for (Billboard* b : bb)
		b->Update();
}

void CpuBillboardDemo::Render()
{
	sky->Render();

	terrain->Render();

	for (Billboard* b : bb)
		b->Render();

}


