#include "stdafx.h"
#include "ViewportDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Terrain.h"
#include "Environment/SkyCube.h"

void ViewportDemo::Initialize()
{

	Context::Get()->GetCamera()->Position(0, 32, -67);
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	dynamic_cast<Freedom*>(Context::Get()->GetCamera())->Speed(50, 5);

	shader = new Shader(L"38_Lighting.fxo");
	sky = new SkyCube(L"Environment/GrassCube1024.dds");

	CreateViewer();

	Mesh();
	Airplane();
	Kachujin();

	AddPointLights();
	AddSpotLights();

}

void ViewportDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(sky);	
}

void ViewportDemo::Update()
{	
	
	static int select = 3;
	ImGui::InputInt("Select", &select);
	select %= 2;
	shader->AsScalar("Select")->SetInt(select);

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
	
	renderTarget2D[0]->Update();
	renderTarget2D[1]->Update();

	
}

void ViewportDemo::PreRender()
{
	//Main
	{
		renderTarget[0]->Set(depthStencil[0]);
		viewport[0]->RSSetViewport();

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

		//airPlane->Render();
		kachujin->Render();

		/*for (int i = 0; i < 4; i++)
		collider[i].Collider->Render(Color(0, 1, 0, 1));*/
	}

	
	//Mini
	{
		renderTarget[1]->Set(depthStencil[1]);
		viewport[1]->RSSetViewport();

		static float x = 0, y = 0, width = D3D::Width(), height = D3D::Height();
		ImGui::SliderFloat("Width", &width, 0, D3D::Width() * 2);
		ImGui::SliderFloat("Height", &height, 0, D3D::Height() * 2);
		ImGui::SliderFloat("X", &x, 0, 250);
		ImGui::SliderFloat("Y", &y, 0, 250);
		viewport[1]->Set(width, height, x, y);
		viewport[1]->RSSetViewport();

		static float fov = 0.5f;
		ImGui::SliderFloat("FOV", &fov, 0, 2);
		Perspective* persfective = Context::Get()->GetPerspective();
		persfective->Set(D3D::Width(), D3D::Height(), 0.1f, 1000, Math::PI * fov);

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

		//airPlane->Render();
		//kachujin->Render();

		/*for (int i = 0; i < 4; i++)
		collider[i].Collider->Render(Color(0, 1, 0, 1));*/
	}
	
}

void ViewportDemo::Render()
{
	renderTarget2D[0]->SRV(renderTarget[0]->SRV());
	renderTarget2D[0]->Render();

	renderTarget2D[1]->SRV(renderTarget[1]->SRV());
	renderTarget2D[1]->Render();
}

void ViewportDemo::CreateViewer()
{
	//Main
	renderTarget[0] = new RenderTarget();
	depthStencil[0] = new DepthStencil();
	viewport[0] = new Viewport(D3D::Width(), D3D::Height());

	renderTarget2D[0] = new Render2D();
	renderTarget2D[0]->GetTransform()->Position(D3D::Width() * 0.5f, D3D::Height() * 0.5f, 0);
	renderTarget2D[0]->GetTransform()->Scale(D3D::Width(), D3D::Height(), 1);

	//Mini
	renderTarget[1] = new RenderTarget();
	depthStencil[1] = new DepthStencil();
	viewport[1] = new Viewport(D3D::Width(), D3D::Height(), 1);

	renderTarget2D[1] = new Render2D();
	renderTarget2D[1]->GetTransform()->Position(125, D3D::Height() - 125 * 0.5f, 0);
	renderTarget2D[1]->GetTransform()->Scale(250, 250, 1);

}

void ViewportDemo::Mesh()
{
	//Create Material
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->SpecularMap("Floor_Specular.png");
		floor->NormalMap("Floor_Normal.png");
		floor->Specular(1, 1, 1, 20);
		floor->Emissive(0.3f, 0.3f, 0.3f, 0.3f);

		stone = new Material(shader);
		stone->DiffuseMap("Stones.png");
		stone->SpecularMap("Stones_Specular.png");
		stone->NormalMap("Stones_Normal.png");
		stone->Specular(1, 1, 1, 20);
		stone->Emissive(0.3f, 0.3f, 0.3f, 0.3f);

		brick = new Material(shader);
		brick->DiffuseMap("Bricks.png");
		brick->SpecularMap("Bricks_Specular.png");
		brick->NormalMap("Bricks_Normal.png");
		brick->Specular(1, 0, 0, 20);
		brick->Emissive(0.3f, 0.3f, 0.3f, 0.3f);

		wall = new Material(shader);
		wall->DiffuseMap("Wall.png");
		wall->SpecularMap("Wall_Specular.png");
		wall->NormalMap("Wall_Normal.png");
		wall->Specular(1, 1, 1, 20);
		wall->Emissive(0.3f, 0.3f, 0.3f, 0.3f);
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
		transform->Scale(12, 1, 12);

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

void ViewportDemo::Airplane()
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

void ViewportDemo::Kachujin()
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
	kachujin->PlayClip(0, 0, 0.25f);

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
	kachujin->PlayClip(3, 3, 0.35f);

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

void ViewportDemo::AddPointLights()
{
	PointLight light;
	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //A
		Color(0.0f, 0.0f, 1.0f, 1.0f), //D
		Color(0.0f, 0.0f, 0.7f, 1.0f), //S
		Color(0.0f, 0.0f, 0.7f, 1.0f), //E
		Vector3(-25, 10, -30), //Position
		5.0f, //Range
		0.9f //Intensity
	};
	Context::Get()->AddPointLight(light);

	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //A
		Color(0.0f, 0.0f, 1.0f, 1.0f), //D
		Color(0.0f, 0.0f, 0.7f, 1.0f), //S
		Color(0.0f, 0.0f, 0.7f, 1.0f), //E
		Vector3(-35, 0.5, -30), //Position
		5.0f, //Range
		0.9f //Intensity
	};
	Context::Get()->AddPointLight(light);


	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //A
		Color(0.0f, 1.0f, 0.0f, 1.0f), //D
		Color(0.0f, 0.0f, 0.7f, 1.0f), //S
		Color(1.0f, 0.0f, 0.0f, 1.0f), //E
		Vector3(-35, 0.5, -17.5), //Position
		5.0f, //Range
		0.9f //Intensity
	};
	Context::Get()->AddPointLight(light);

	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //A
		Color(0.0f, 0.0f, 1.0f, 1.0f), //D
		Color(0.0f, 0.0f, 0.7f, 1.0f), //S
		Color(1.0f, 0.0f, 0.7f, 1.0f), //E
		Vector3(-5, 1, -17.5), //Position
		5.0f, //Range
		0.9f //Intensity
	};
	Context::Get()->AddPointLight(light);

	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //A
		Color(1.0f, 0.0f, 0.0f, 1.0f), //D
		Color(0.0f, 0.7f, 0.0f, 1.0f), //S
		Color(1.0f, 0.7f, 0.0f, 1.0f), //E
		Vector3(-10, 1, -17.5), //Position
		5.0f, //Range
		0.9f //Intensity
	};
	Context::Get()->AddPointLight(light);

	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //A
		Color(0.0f, 0.0f, 1.0f, 1.0f), //D
		Color(0.0f, 0.2f, 0.0f, 1.0f), //S
		Color(0.0f, 1.0f, 0.0f, 0.2f), //E
		Vector3(25, 10, -30), //Position
		10.0f, //Range
		0.3f //Intensity
	};
	Context::Get()->AddPointLight(light);


}

void ViewportDemo::AddSpotLights()
{
	SpotLight light;
	light =
	{
		Color(0.3f, 1.0f, 0.0f, 1.0f),
		Color(0.7f, 1.0f, 0.0f, 1.0f),
		Color(0.3f, 1.0f, 0.0f, 1.0f),
		Color(0.3f, 1.0f, 0.0f, 1.0f),
		Vector3(-10, 20, -30),
		25.0f,
		Vector3(0, -1, 0),
		30.0f,
		0.9f
	};
	Context::Get()->AddSpotLight(light);

	light =
	{
		Color(1.0f, 0.2f, 0.9f, 1.0f),
		Color(1.0f, 0.2f, 0.9f, 1.0f),
		Color(1.0f, 0.2f, 0.9f, 1.0f),
		Color(1.0f, 0.2f, 0.9f, 1.0f),
		Vector3(10, 20, -30),
		30.0f,
		Vector3(0, -1, 0),
		40.0f,
		0.9f
	};
	Context::Get()->AddSpotLight(light);

	
}

void ViewportDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}


