#include "stdafx.h"
#include "FrameworkDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Terrain.h"

void FrameworkDemo::Initialize()
{
	Context::Get()->GetCamera()->Position(0, 32, -67);
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	dynamic_cast<Freedom*>(Context::Get()->GetCamera())->Speed(50, 5);

	shader = new Shader(L"13_Mesh.fx");

	Transform* transform = NULL;

	cube = new MeshRender(shader, new MeshCube());
	transform = cube->AddTransform();

	transform->Position(0, 5, 0);
	transform->Scale(20, 10, 20);
	cube->UpdateTransforms();

	grid = new MeshRender(shader, new MeshGrid(5, 5));
	transform = grid->AddTransform();
	transform->Scale(20, 1, 20);
	grid->UpdateTransforms();

	cylider = new MeshRender(shader, new MeshCylinder(0.5f, 3.0f, 20, 20));
	sphere = new MeshRender(shader, new MeshSphere(0.5f, 20, 20));
	for (UINT i = 0; i < 5; i++)
	{
		//Cylinder
		cylider->AddTransform();
		cylider->GetTransform(i)->Position(-30, 6, -15.0f + (float)i * 15.0f);
		cylider->GetTransform(i)->Scale(5, 5, 5);

		cylider->AddTransform();
		cylider->GetTransform(i)->Position(30, 6, -15.0f + (float)i * 15.0f);
		cylider->GetTransform(i)->Scale(5, 5, 5);

		//Sphere
		sphere->AddTransform();
		sphere->GetTransform(i)->Position(-30, 15.5, -15.0f + (float)i * 15.0f);
		sphere->GetTransform(i)->Scale(5, 5, 5);

		sphere->AddTransform();
		sphere->GetTransform(i)->Position(30, 15.5, -15.0f + (float)i * 15.0f);
		sphere->GetTransform(i)->Scale(5, 5, 5);
	}

	cylider->UpdateTransforms();
	sphere->UpdateTransforms();

	terrainShader = new Shader(L"12_Terrain.fx");
	terrain = new Terrain(terrainShader, L"Terrain/Gray256.png");
	terrain->BaseMap(L"Terrain/Dirt3.png");
	
}

void FrameworkDemo::Destroy()
{
	SafeDelete(shader);
	
	SafeDelete(grid);
	SafeDelete(cube);

	SafeDelete(cylider);
	SafeDelete(sphere);

	SafeDelete(terrainShader);
	SafeDelete(terrain);
	
}

void FrameworkDemo::Update()
{	
	cylider->Update();
	sphere->Update();
	cube->Update();
	grid->Update();
	terrain->Update();
	
}

void FrameworkDemo::Render()
{
	cylider->Render();
	sphere->Render();

	grid->Render();
	cube->Render();

	terrain->Render();
}


