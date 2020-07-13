#include "stdafx.h"
#include "GetHeightDemo.h"
#include "Environment/Terrain.h"
#include "Viewer/Freedom.h"

void GetHeightDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"12_Terrain.fx");
	
	terrain = new Terrain(shader, L"Terrain/Gray256.png");
	terrain->BaseMap(L"Terrain/Dirt3.png");

	meshShader = new Shader(L"10_Mesh.fx");
	sphere = new MeshRender(meshShader, new MeshSphere(0.5f));
	sphere->AddTransform();
	sphere->UpdateTransforms();
	
}

void GetHeightDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(terrain);

	SafeDelete(meshShader);
	SafeDelete(sphere);
}

void GetHeightDemo::Update()
{
	Vector3 position;
	sphere->GetTransform(0)->Position(&position);

	if (Keyboard::Get()->Press(VK_UP))
		position.z += 20.0f * Time::Delta();
	else if (Keyboard::Get()->Press(VK_DOWN))
		position.z -= 20.0f * Time::Delta();

	if (Keyboard::Get()->Press(VK_RIGHT))
		position.x += 20.0f * Time::Delta();
	else if (Keyboard::Get()->Press(VK_LEFT))
		position.x -= 20.0f * Time::Delta();

	position.y = terrain->GetHeight(position);
	sphere->GetTransform(0)->Position(position);

	static Vector3 light = Vector3(-1, -1, 1);
	ImGui::SliderFloat3("Light", light, -1, 1);
	shader->AsVector("LightDirection")->SetFloatVector(light);

	sphere->Update();
	terrain->Update();
}

void GetHeightDemo::Render()
{
	//terrain->Pass(1);
	terrain->Render();
	sphere->Render();

}


