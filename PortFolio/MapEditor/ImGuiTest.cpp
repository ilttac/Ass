#include "stdafx.h"
#include "ImGuiTest.h"

void ImGuiTest::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(10, 0, 0);
	Context::Get()->GetCamera()->Position(0, 18, -78);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 2);

	shader = new Shader(L"54_Water.fxo");

	shadow = new Shadow(shader, Vector3(0, 0, 0), 65);

	sky = new Sky(shader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.3f);

	Mesh();
}

void ImGuiTest::Destroy()
{
	SafeDelete(shader);
	SafeDelete(shadow);

	SafeDelete(sky);
}

void ImGuiTest::Update()
{
	UINT& type = Context::Get()->FogType();
	ImGui::InputInt("FogType", (int*)& type);
	type %= 3;

	ImGui::ColorEdit3("FogColor", Context::Get()->FogColor());
	ImGui::SliderFloat("FogMin", &Context::Get()->FogDistance().x, 0.0f, 10.0f);
	ImGui::SliderFloat("FogMax", &Context::Get()->FogDistance().y, 0.0f, 300.0f);
	ImGui::SliderFloat("FogDensity", &Context::Get()->FogDensity(), 0.0f, 300.0f);

	sphere->Update();
	grid->Update();
	sky->Update();
}

void ImGuiTest::PreRender()
{
	sky->PreRender();

	//Depth
	{
		shadow->Set();

		Pass(0, 1, 2);

		//sky->Pass(0);
		//sky->Render();


		sphere->Render();
		floor->Render();
	}

	//Reflection
	{
		sky->Pass(10, 11, 12);
		sky->Render();

		Pass(13, 14, 15);

		sphere->Render();
		floor->Render();
		grid->Render();
	}

	//Refraction
	{

		sky->Pass(4, 5, 6);
		sky->Render();

		Pass(7, 8, 9);
		floor->Render();
		grid->Render();

	}
}

void ImGuiTest::Render()
{
	sky->Pass(4, 5, 6);
	sky->Render();


	Pass(7, 8, 9);

	sphere->Render();
	grid->Render();
	floor->Render();
}

void ImGuiTest::PostRender()
{
	//gBuffer->DebugRender();
	//sky->PostRender();
}

void ImGuiTest::Mesh()
{
	//Create Material
	{
		//¹Ù´Ú
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->SpecularMap("Floor_Specular.png");
		floor->NormalMap("Floor_Normal.png");
		floor->Specular(1, 1, 1, 15);
		floor->Emissive(0.2f, 0.2f, 0.2f, 0.3f);
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		sphere = new MeshRender(shader, new MeshSphere(0.5f, 20, 20));

		grid = new MeshRender(shader, new MeshGrid(15, 15));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 1, 20);

		for (UINT i = 0; i < 5; i++)
		{

			transform = sphere->AddTransform();
			transform->Position(-30, 15.5f, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			transform = sphere->AddTransform();
			transform->Position(30, 15.5f, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);
		}
	}
	grid->UpdateTransforms();
	sphere->UpdateTransforms();
	meshes.push_back(sphere);
	meshes.push_back(grid);
}

void ImGuiTest::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);
	for (ModelRender* temp : models)
		temp->Pass(model);
	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}
