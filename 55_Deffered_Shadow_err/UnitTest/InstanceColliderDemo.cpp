#include "stdafx.h"
#include "InstanceColliderDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Terrain.h"
#include "Environment/SkyCube.h"

void InstanceColliderDemo::Initialize()
{
	Context::Get()->GetCamera()->Position(0, 32, -67);
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	dynamic_cast<Freedom*>(Context::Get()->GetCamera())->Speed(50, 5);

	shader = new Shader(L"33_Animation.fx");
	sky = new SkyCube(L"Environment/GrassCube1024.dds");

	Mesh();
	Airplane();
	Kachujin();
}

void InstanceColliderDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(sky);	
}

void InstanceColliderDemo::Update()
{	
	sky->Update();

	sphere->Update();
	cylinder->Update();
	cube->Update();
	grid->Update();

	airPlane->Update();
	kachujin->Update();


	for (int i = 0; i < 4; i++)
	{
		Matrix attach = kachujin->GetAttachTransform(i);
		collider[i].Collider->GetTransform()->World(attach);
		collider[i].Collider->Update();
	}
	
	
	
}

void InstanceColliderDemo::Render()
{
	sky->Render();

	Pass(0, 1, 2);

	wall->Render();
	sphere->Render();

	brick->Render();
	cylinder->Render();

	stone->Render();
	cube->Render();

	floor->Render();
	grid->Render();

	airPlane->Render();
	kachujin->Render();

	for (int i = 0; i < 4; i++)
		collider[i].Collider->Render(Color(0, 1, 0, 1));
}

void InstanceColliderDemo::Mesh()
{
	//Create Material
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");

		stone = new Material(shader);
		stone->DiffuseMap("Stones.png");

		brick = new Material(shader);
		brick->DiffuseMap("Bricks.png");

		wall = new Material(shader);
		wall->DiffuseMap("Wall.png");
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		cube = new MeshRender(shader, new MeshCube());
		transform = cube->AddTransform();
		transform->Position(0, 5, 0);
		transform->Scale(20, 10, 20);

		grid = new MeshRender(shader, new MeshGrid(5, 5));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 10, 20);

		cylinder = new MeshRender(shader, new MeshCylinder(0.5f, 3.0f, 20, 20));
		sphere = new MeshRender(shader, new MeshSphere(0.5f, 20, 20));
		for (UINT i = 0; i < 5; i++)
		{
			transform = cylinder->AddTransform();
			transform->Position(-30, 6, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			transform = cylinder->AddTransform();
			transform->Position(30, 6, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			transform = sphere->AddTransform();
			transform->Position(-30, 15.5f, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			transform = sphere->AddTransform();
			transform->Position(30, 15.5f, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);
		}
	}

	sphere->UpdateTransforms();
	cylinder->UpdateTransforms();
	cube->UpdateTransforms();
	grid->UpdateTransforms();

	meshes.push_back(sphere);
	meshes.push_back(cylinder);
	meshes.push_back(cube);
	meshes.push_back(grid);
}

void InstanceColliderDemo::Airplane()
{
	airPlane = new ModelRender(shader);
	airPlane->ReadMaterial(L"B787/Airplane");
	airPlane->ReadMesh(L"B787/Airplane");
	
	Transform* transform = airPlane->AddTransform();
	transform->Position(2.0f, 9.91f, 2.0f);	
	transform->Scale(0.004f, 0.004f, 0.004f);
	airPlane->UpdateTransforms();

	models.push_back(airPlane);
}

void InstanceColliderDemo::Kachujin()
{
	weapon = new Model();
	weapon->ReadMaterial(L"Weapon/Sword");
	weapon->ReadMesh(L"Weapon/Sword");

	kachujin = new ModelAnimator(shader);
	kachujin->ReadMaterial(L"Kachujin/Mesh");
	kachujin->ReadMesh(L"Kachujin/Mesh");
	kachujin->ReadClip(L"Kachujin/Idle");
	kachujin->ReadClip(L"Kachujin/Running");
	kachujin->ReadClip(L"Kachujin/Jump");
	kachujin->ReadClip(L"Kachujin/Hip_Hop_Dancing");

	Transform attachTransform;

	attachTransform.Position(-10, 0, -10);
	attachTransform.Scale(0.5f, 0.5f, 0.5f);

	kachujin->GetModel()->Attach(shader, weapon, 35, &attachTransform);

	Transform* transform = NULL;

	transform = kachujin->AddTransform();
	transform->Position(-25, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(0, 0, 1.0f);

	transform = kachujin->AddTransform();
	transform->Position(-10, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(1, 1, 1.0f);

	transform = kachujin->AddTransform();
	transform->Position(10, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(2, 2, 1.0f);

	transform = kachujin->AddTransform();
	transform->Position(25, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(3, 3, 1.0f);

	kachujin->UpdateTransforms();

	animators.push_back(kachujin);

	for (UINT i = 0; i < 4; i++)
	{
		collider[i].Init = new Transform();

		collider[i].Init->Scale(10, 10, 50);
		collider[i].Init->Position(-10, 0, -40);

		collider[i].Transform = new Transform();
		collider[i].Collider = new Collider(collider[i].Transform, collider[i].Init);
	}
}

void InstanceColliderDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}


