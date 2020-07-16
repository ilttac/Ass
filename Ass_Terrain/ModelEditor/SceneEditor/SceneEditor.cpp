#include "stdafx.h"
#include "SceneEditor.h"
#include "Utilities/Xml.h"

void SceneEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(32, 0, 0);
	Context::Get()->GetCamera()->Position(36, 170, -310);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"55_TerrainLod.fxo");
	//skyShader = new Shader(L"57_ParticleViewer.fxo");

	//shadow = new Shadow(skyShader, Vector3(0, 0, 0), 65);
	//Terrain

	terrainInitialDesc =
	{
		shader,
		L"Terrain/Gray1024.jpg",
		1.0f,
		16,
		2
	};

	terrainLod = new TerrainLod(terrainInitialDesc);
	terrainLod->BaseMap(L"Terrain/Dirt.png");
	terrainLod->LayerMap(L"Terrain/Dirt3.png", L"Terrain/Gray1024.jpg");
	terrainLod->NormalMap(L"Terrain/Gray1024_Normal.png");
	/*sky = new Sky(skyShader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.5f);*/
}

void SceneEditor::Destroy()
{
	SafeDelete(shader);
	SafeDelete(shadow);

	SafeDelete(sky);

	SafeDelete(terrainLod);
}

void SceneEditor::Update()
{

	//sky->Update();

	static Vector3 light = Vector3(-1, -1, 1);
	ImGui::SliderFloat3("Light", light, -1, 1);
	shader->AsVector("LightDirection")->SetFloatVector(light);

	terrainLod->Update();
}

void SceneEditor::PreRender()
{
	//sky->PreRender();

	////Depth
	//{
	//	shadow->Set();
	//	//sphere->Render();
	//}
}

void SceneEditor::Render()
{
//	sky->Pass(4, 5, 6);
	//sky->Render();

	static bool bCheck = false;
	ImGui::Checkbox("WireFrame", &bCheck);
	terrainLod->Pass(bCheck ? 1 : 0);

	//terrain->Pass(1);
	terrainLod->Render();
}
