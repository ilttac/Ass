#include "stdafx.h"
#include "DrawModel.h"
#include "Viewer/Freedom.h"

void DrawModel::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(0, 0, 0);
	Context::Get()->GetCamera()->Position(0, 0, -50);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 2);

	shader = new Shader(L"32_Model.fx");

	Airplane();
	Tower();
	Tank();
}

void DrawModel::Update()
{
	if(airPlane != NULL) airPlane->Update();
	if(tower != NULL) tower->Update();
	if (tank != NULL) tank->Update();	
}

void DrawModel::Render()
{
	
	if (airPlane != NULL) airPlane->Render();
	if (tower != NULL) tower->Render();
	if (tank != NULL) tank->Render();
}

void DrawModel::Airplane()
{
	airPlane = new ModelRender(shader);
	airPlane->ReadMaterial(L"B787/Airplane");
	airPlane->ReadMesh(L"B787/Airplane");	

	for (float x = -50; x < 50; x += 2.5f)
	{
		Transform* transform = airPlane->AddTransform();

		transform->Position(Vector3(x, 0.0f, 2.5f));
		transform->RotationDegree(0, Math::Random(-180.0f, 180.0f), 0);
		transform->Scale(0.00025f, 0.00025f, 0.00025f);
	}
	airPlane->UpdateTransforms();	

	airPlane->Pass(1);
}

void DrawModel::Tower()
{
	tower = new ModelRender(shader);
	tower->ReadMaterial(L"Tower/Tower");
	tower->ReadMesh(L"Tower/Tower");

	for (float x = -50; x < 50; x += 2.5f)
	{
		Transform* transform = tower->AddTransform();
		transform->Position(x, 0, 7.5f);
		transform->RotationDegree(0, Math::Random(-180.0f, 180.0f), 0);
		transform->Scale(0.003f, 0.003f, 0.003f);
	}
	tower->UpdateTransforms();
	

	tower->Pass(1);
}

void DrawModel::Tank()
{
	tank = new ModelRender(shader);
	tank->ReadMaterial(L"Tank/Tank");
	tank->ReadMesh(L"Tank/Tank");
	
	for (float x = -50; x < 50; x += 2.5f)
	{
		Transform* transform = tank->AddTransform();
		transform->Position(x, 0, 5.0f);
		transform->RotationDegree(0, Math::Random(-180.0f, 180.0f), 0);
		transform->Scale(0.1f, 0.1f, 0.1f);
	}
	tank->UpdateTransforms();	

	tank->Pass(1);
}


