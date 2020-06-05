#include "stdafx.h"
#include "ScatteringDemo.h"

void ScatteringDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(10, 0, 0);
	Context::Get()->GetCamera()->Position(0, 18, -78);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);

	shader = new Shader(L"53_DefferedShadow.fxo");

	shadow = new Shadow(shader, Vector3(0, 0, 0), 65);
	gBuffer = new GBuffer(shader);

	sky = new Sky(shader);
	sky->ScatteringPass(11);
	sky->Pass(12, 13, 14);
	sky->Theata(Math::PI - 1e-6f);
	sky->RealTime(true, Math::PI - 1e-6f, 0.3f);

	snow = new Snow(Vector3(300, 100, 500), 1000, L"Environment/Snow.png");

	Mesh();
	Airplane();
	Kachujin();

	AddPointLights();
	AddSpotLights();

}

void ScatteringDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(gBuffer);
	SafeDelete(shadow);

	SafeDelete(sky);
	SafeDelete(snow);
}

void ScatteringDemo::Update()
{
	UINT& type = Context::Get()->FogType();
	ImGui::InputInt("FogType", (int*)&type);
	type %= 3;
	
	ImGui::ColorEdit3("FogColor", Context::Get()->FogColor());
	ImGui::SliderFloat("FogMin", &Context::Get()->FogDistance().x, 0.0f, 10.0f);
	ImGui::SliderFloat("FogMax", &Context::Get()->FogDistance().y, 0.0f, 300.0f);
	ImGui::SliderFloat("FogDensity", &Context::Get()->FogDensity(), 0.0f, 300.0f);
	

	sphere->Update();	
	cylinder->Update();
	cube->Update();
	grid->Update();

	airplane->Update();
	kachujin->Update();

	/*for (int i = 0; i < 4; i++)
	{
		Matrix attach = kachujin->GetAttachTransform(i);
		colliders[i].Collider->GetTransform()->World(attach);
		colliders[i].Collider->Update();
	}*/

	
	
	sky->Update();
	snow->Udpate();

}

void ScatteringDemo::PreRender()
{
	//Depth
	{
		shadow->Set();

		Pass(0, 1, 2);

		//sky->Pass(0);
		//sky->Render();

		wall->Render();
		sphere->Render();

		brick->Render();
		cylinder->Render();

		stone->Render();
		cube->Render();

		floor->Render();
		grid->Render();

		airplane->Render();
		kachujin->Render();
	}

	//GBuffer
	{
		gBuffer->PackGBuffer();

		Pass(3, 4, 5);

		//sky->Pass(0);
		//sky->Render();

		wall->Render();
		sphere->Render();

		brick->Render();
		cylinder->Render();

		stone->Render();
		cube->Render();

		floor->Render();
		grid->Render();

		airplane->Render();
		kachujin->Render();
	}

	sky->PreRender();

}

void ScatteringDemo::Render()
{
	gBuffer->Render();	
	sky->Render();

	snow->Render();
}

void ScatteringDemo::PostRender()
{
	//gBuffer->DebugRender();
	sky->PostRender();
}

void ScatteringDemo::Mesh()
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

		//Àç´Ü
		stone = new Material(shader);
		stone->DiffuseMap("Stones.png");
		stone->SpecularMap("Stones_Specular.png");
		stone->NormalMap("Stones_Normal.png");
		stone->Specular(1, 1, 1, 2);
		stone->Emissive(0.2f, 0.2f, 0.2f, 0.3f);

		//±âµÕ
		brick = new Material(shader);
		brick->DiffuseMap("Bricks.png");
		brick->SpecularMap("Bricks_Specular.png");
		brick->NormalMap("Bricks_Normal.png");
		brick->Specular(0.3f, 0.3f, 0.3f, 20.0f);
		brick->Emissive(0.2f, 0.2f, 0.2f, 0.3f);

		//±¸
		wall = new Material(shader);
		wall->DiffuseMap("Wall.png");
		wall->SpecularMap("Wall_Specular.png");
		wall->NormalMap("Wall_Normal.png");
		wall->Specular(1, 1, 1, 20);
		wall->Emissive(0.2f, 1.0f, 0.2f, 0.5f);

	}

	//Create Mesh
	{
		Transform* transform = NULL;

		cube = new MeshRender(shader, new MeshCube());
		transform = cube->AddTransform();
		transform->Position(0, 5, 0);
		transform->Scale(20, 10, 20);

		grid = new MeshRender(shader, new MeshGrid(15, 15));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 1, 20);

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

void ScatteringDemo::Airplane()
{
	airplane = new ModelRender(shader);
	airplane->ReadMaterial(L"B787/Airplane");
	airplane->ReadMesh(L"B787/Airplane");

	Transform* transform = airplane->AddTransform();
	transform->Position(2.0f, 9.91f, 2.0f);
	transform->Scale(0.004f, 0.004f, 0.004f);
	airplane->UpdateTransforms();

	models.push_back(airplane);
}

void ScatteringDemo::Kachujin()
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
	attachTransform.Scale(1.0f, 1.0f, 1.0f);

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
	kachujin->PlayClip(2, 2, 0.75f);

	transform = kachujin->AddTransform();
	transform->Position(25, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(3, 3, 0.35f);

	kachujin->UpdateTransforms();

	animators.push_back(kachujin);

	for (UINT i = 0; i < 4; i++)
	{
		colliders[i].Init = new Transform();

		colliders[i].Init->Scale(10, 10, 120);
		colliders[i].Init->Position(-10, 0, -60);

		colliders[i].Transform = new Transform();
		colliders[i].Collider = new Collider(colliders[i].Transform, colliders[i].Init);
	}
}

void ScatteringDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}

void ScatteringDemo::AddPointLights()
{
	PointLight light;	

	for (int z = -30; z <= 30; z += 30)
	{
		for (int x = -30; x <= 30; x += 30)
		{
			light =
			{
				Color(0.0f, 0.0f, 0.0f, 1.0f), //A
				Math::RandomColor3(), //D				
				Color(0.0f, 0.0f, 0.0f, 1.0f), //S
				Color(0.0f, 0.0f, 0.0f, 1.0f), //E
				Vector3(x, 1, z), //Position
				5.0f, //Range
				Math::Random(0.1f, 1.0f) //Intensity
			};

			Context::Get()->AddPointLight(light);
		}
		
	}
}

void ScatteringDemo::AddSpotLights()
{
	SpotLight light;
	light =
	{
		Color(0.3f, 1.0f, 0.0f, 1.0f), //A
		Color(0.7f, 1.0f, 0.0f, 1.0f), //D
		Color(0.3f, 1.0f, 0.0f, 1.0f), //S
		Color(0.3f, 1.0f, 0.0f, 1.0f), //E
		Vector3(-10, 20, -30), //Position
		25.0f, //Range
		Vector3(0, -1, 0), //Direction
		30.0f, //Angle
		0.9f //Intensity
	};
	Context::Get()->AddSpotLight(light);

	light =
	{
		Color(1.0f, 0.2f, 0.9f, 1.0f), //A
		Color(1.0f, 0.2f, 0.9f, 1.0f), //D
		Color(1.0f, 0.2f, 0.9f, 1.0f), //S
		Color(1.0f, 0.2f, 0.9f, 1.0f), //E
		Vector3(10, 20, -30), //Position
		30.0f, //Range
		Vector3(0, -1, 0), //Direction
		40.0f,//Angle
		0.9f//Intensity
	};
	Context::Get()->AddSpotLight(light);
}


