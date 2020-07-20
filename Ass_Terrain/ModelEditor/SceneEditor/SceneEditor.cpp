#include "stdafx.h"
#include "SceneEditor.h"
#include "Utilities/Xml.h"

void SceneEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"23_TerrainSpatting.fxo");
	//skyShader = new Shader(L"57_ParticleViewer.fxo");

	//shadow = new Shadow(skyShader, Vector3(0, 0, 0), 65);
	
	//Terrain
	terrain = new Terrain(shader, L"Terrain/TestMap.png");
	terrain->BaseMap(L"Terrain/Dirt3.png");
	terrain->LayerMap(L"Terrain/Cliff (Layered Rock).jpg", L"Terrain/Splatting.png");
	
	/*sky = new Sky(skyShader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.5f);*/
}

void SceneEditor::Destroy()
{
	SafeDelete(shader);
	SafeDelete(shadow);

	SafeDelete(sky);

	SafeDelete(terrain);
}

void SceneEditor::Update()
{

	//sky->Update();
	terrain->Update();

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
	terrain->Render();
}
