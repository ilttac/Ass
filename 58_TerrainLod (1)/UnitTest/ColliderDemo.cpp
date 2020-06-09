#include "stdafx.h"
#include "ColliderDemo.h"
#include "Environment/Terrain.h"
#include "Viewer/Freedom.h"

void ColliderDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 3, -8);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 2);

	shader = new Shader(L"25_Model.fx");
	
	gridMaterial = new Material(shader);
	gridMaterial->DiffuseMap(L"Floor.png");
	grid = new MeshRender(shader, new MeshGrid());
	grid->AddTransform();
	grid->UpdateTransforms();

	t = new Transform();
	t->Position(0, 0.5f, 0);
	t->Scale(2, 1, 1);
	collider = new Collider(t);

	t2 = new Transform();
	t2->Position(2, 0.5f, 0);
	collider2 = new Collider(t2);
}

void ColliderDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(gridMaterial);
	SafeDelete(grid);

	SafeDelete(t);
	SafeDelete(t2);

	SafeDelete(collider);
	SafeDelete(collider2);
}

void ColliderDemo::Update()
{
	grid->Update();

	D3DXVECTOR3 position2;
	t2->Position(&position2);

	if (Keyboard::Get()->Press(VK_SHIFT) == false)
	{
		if (Keyboard::Get()->Press(VK_RIGHT))
			position2.x += 3.0f * Time::Delta();
		else if (Keyboard::Get()->Press(VK_LEFT))
			position2.x -= 3.0f * Time::Delta();

		if (Keyboard::Get()->Press(VK_UP))
			position2.z += 3.0f * Time::Delta();
		else if (Keyboard::Get()->Press(VK_DOWN))
			position2.z -= 3.0f * Time::Delta();
	}
	t2->Position(position2);

	D3DXVECTOR3 rotation = D3DXVECTOR3(0, 0, 0);
	D3DXVECTOR3 rotation2 = D3DXVECTOR3(0, 0, 0);

	t->RotationDegree(&rotation);
	t2->RotationDegree(&rotation2);

	ImGui::SliderFloat3("Rotation", (float*)rotation, -179, 179);
	ImGui::SliderFloat3("Rotation2", (float*)rotation2, -179, 179);

	t->RotationDegree(rotation);
	t2->RotationDegree(rotation2);

	collider->Update();
	collider2->Update();
}

void ColliderDemo::Render()
{

	//gridMaterial->Render();
	grid->Render();

	bool bCheck = collider->IsIntersect(collider2);
	collider->Render(bCheck ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));
	collider2->Render(bCheck ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1));

}


