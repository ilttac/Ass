#include "stdafx.h"
#include "TerrainLodDemo.h"


void TerrainLodDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(32, 0, 0);
	Context::Get()->GetCamera()->Position(36, 170, -310);
	((Freedom *)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"55_TerrainLod.fxo");

	//Terrain
	{
		TerrainLod::InitialDesc desc =
		{
			shader,
			L"Terrain/Gray1024.jpg",
			1.0f,
			16,
			2
		};

		terrain = new TerrainLod(desc);
		terrain->BaseMap(L"Terrain/Dirt.png");
		terrain->LayerMap(L"Terrain/Dirt3.png", L"Terrain/Gray1024.jpg");
		terrain->NormalMap(L"Terrain/Dirt_Normal.png");
		//terrain->Pass(1);
	}

	
	
}

void TerrainLodDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(terrain);
}

void TerrainLodDemo::Update()
{
	static Vector3 light = Vector3(-1, -1, 1);
	ImGui::SliderFloat3("Light", light, -1, 1);
	shader->AsVector("LightDirection")->SetFloatVector(light);

	terrain->Update();
}

void TerrainLodDemo::Render()
{
	static bool bCheck = false;
	ImGui::Checkbox("WireFrame", &bCheck);
	terrain->Pass(bCheck ? 1 : 0);

	//terrain->Pass(1);
	terrain->Render();

}


