#include "stdafx.h"
#include "BloomDemo.h"

void BloomDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(10, 0, 0);
	Context::Get()->GetCamera()->Position(0, 18, -78);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 2);

	shader = new Shader(L"54_Water.fxo");

	shadow = new Shadow(shader, Vector3(0, 0, 0), 65);

	sky = new Sky(shader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.3f);

	snow = new Snow(Vector3(300, 100, 500), 1000, L"Environment/Snow.png");

	float width = D3D::Width(), height = D3D::Height();
	renderTarget[0] = new RenderTarget((UINT)width, (UINT)height); //Diffuse
	renderTarget[1] = new RenderTarget((UINT)width, (UINT)height); //Luminosity
	renderTarget[2] = new RenderTarget((UINT)width, (UINT)height); //BlurX
	renderTarget[3] = new RenderTarget((UINT)width, (UINT)height); //BlurY
	renderTarget[4] = new RenderTarget((UINT)width, (UINT)height); //Composite
	renderTarget[5] = new RenderTarget((UINT)width, (UINT)height); //ColorGrading

	depthStencil = new DepthStencil((UINT)width, (UINT)height);
	viewPort = new Viewport(width, height);

	render2D = new Render2D();
	render2D->GetTransform()->Position(200, 120, 0);
	render2D->GetTransform()->Scale(355, 200, 1);
	render2D->SRV(renderTarget[0]->SRV());

	postEffect = new PostEffect(L"62_Bloom.fxo");
	billShader = new Shader(L"56_Billboard.fxo");
	AddBillboard();

	Mesh();
	Airplane();
	Kachujin();

	AddPointLights();
	AddSpotLights();

}

void BloomDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(shadow);

	SafeDelete(sky);
	SafeDelete(snow);

	for (UINT i = 0; i < 6; i++)
	{
		SafeDelete(renderTarget[i]);
	}
	SafeDelete(depthStencil);
	SafeDelete(viewPort);
	SafeDelete(render2D);
	SafeDelete(postEffect);

}

void BloomDemo::Update()
{
	//Theta
	{
		static float theta = Math::PI - 1e-6f;
		ImGui::SliderFloat("Light", &theta, -Math::PI - 1e-6f, +Math::PI - 1e-6f);
		sky->Theata(theta);
	}

	//Bloom
	{
		ImGui::Checkbox("Origin", &bOrigin);

		ImGui::InputFloat("Threshold", &threshold, 0.01f);
		postEffect->GetShader()->AsScalar("Threshold")->SetFloat(threshold);

		ImGui::InputInt("BlurCount", (int*)&blurCount, 2);
		blurCount = Math::Clamp<UINT>(blurCount, 1, 33);

		postEffect->GetShader()->AsScalar("BlurCount")->SetInt(blurCount);
	}

	SetBlur();


	sphere->Update();
	cylinder->Update();
	cube->Update();
	grid->Update();

	airplane->Update();
	kachujin->Update();

	sky->Update();
	snow->Udpate();

	billboard->Update();
	postEffect->Update();
}

void BloomDemo::PreRender()
{
	sky->PreRender();

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

	//MainRender
	{
		viewPort->RSSetViewport();
		renderTarget[0]->Set(depthStencil);

		sky->Pass(4, 5, 6);
		sky->Render();

		Pass(7, 8, 9);

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

		snow->Render();

	/*	billboard->Pass(1);
		billboard->Render();*/
	}
	Vector2 PixelSize = Vector2(1.0f / D3D::Width(), 1.0f / D3D::Height());
	postEffect->GetShader()->AsVector("PixelSize")->SetFloatVector(PixelSize);

	//Luminosity
	{
		renderTarget[1]->Set(depthStencil);

		postEffect->Pass(1);
		postEffect->SRV(renderTarget[0]->SRV());
		postEffect->Render();
	}

	//BlurX
	{
		renderTarget[2]->Set(depthStencil);
		viewPort->RSSetViewport();

		postEffect->GetShader()->AsScalar("Weights")->SetFloatArray(&weightX[0], 0, weightX.size());
		postEffect->GetShader()->AsVector("Offsets")->SetRawValue(&offsetX[0], 0, sizeof(Vector2)*offsetX.size());
	
		postEffect->SRV(renderTarget[1]->SRV());
		postEffect->Pass(2);
		postEffect->Render();
	}

	//BlurX
	{
		renderTarget[3]->Set(depthStencil);
		viewPort->RSSetViewport();

		postEffect->GetShader()->AsScalar("Weights")->SetFloatArray(&weightY[0], 0, weightY.size());
		postEffect->GetShader()->AsVector("Offsets")->SetRawValue(&offsetY[0], 0, sizeof(Vector2) * offsetY.size());

		postEffect->SRV(renderTarget[2]->SRV());
		postEffect->Pass(2);
		postEffect->Render();
	}
	//Composite
	{
		renderTarget[4]->Set(depthStencil);
		viewPort->RSSetViewport();

		postEffect->GetShader()->AsSRV("LuminosityMap")->SetResource(renderTarget[1]->SRV());
		postEffect->GetShader()->AsSRV("BlurMap")->SetResource(renderTarget[3]->SRV());

		postEffect->Pass(3);
		postEffect->Render();
	}

	//Color Grading
	{
		renderTarget[5]->Set(depthStencil);
		viewPort->RSSetViewport();

		postEffect->Pass(4);
		postEffect->SRV(renderTarget[4]->SRV());
		postEffect->Render();
	}
	
}

void BloomDemo::Render()
{
	ImGui::InputInt("Target Index", (int*)& targetIndex);
	targetIndex = Math::Clamp<UINT>(targetIndex, 0, 5);

	postEffect->Pass(0);
	postEffect->SRV(renderTarget[targetIndex]->SRV());
	postEffect->Render();

	render2D->Render();
}

void BloomDemo::PostRender()
{
	//gBuffer->DebugRender();
	//sky->PostRender();

	if (Keyboard::Get()->Down('P'))
		renderTarget[2]->SaveTexture(L"../Screenshot.png");
}

void BloomDemo::Mesh()
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

void BloomDemo::Airplane()
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

void BloomDemo::Kachujin()
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


	Transform * transform = NULL;

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

void BloomDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}

void BloomDemo::AddPointLights()
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
		Color(0.0f, 0.2f, 0.0f, 1.0f), //S
		Color(0.0f, 1.0f, 0.0f, 0.2f), //E
		Vector3(25, 10, -30), //Position
		10.0f, //Range
		0.3f //Intensity
	};
	Context::Get()->AddPointLight(light);
}

void BloomDemo::AddSpotLights()
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

void BloomDemo::AddBillboard()
{
	billboard = new Billboard(billShader);

	billboard->AddTexture(L"Terrain/grass_07.tga");


	for (UINT i = 0; i < 3500; i++)
	{
		Vector2 scale = Math::RandomVec2(5, 10);
		Vector2 position = Math::RandomVec2(-80, 80);

		billboard->Add(Vector3(position.x, scale.y * 0.5f, position.y), scale, 0);
	}


}

void BloomDemo::SetBlur()
{
	float x = 1.0f / D3D::Width();
	float y = 1.0f / D3D::Height();

	GetBlurParameter(weightX, offsetX, x, 0);
	GetBlurParameter(weightY, offsetY, 0, y);

}

void BloomDemo::GetBlurParameter(vector<float>& weights, vector<Vector2>& offsets, float x, float y)
{
	weights.clear();
	weights.assign(blurCount, float());

	offsets.clear();
	offsets.assign(blurCount, Vector2());

	weights[0] = GetGaussianFuntion(0);
	offsets[0] = Vector2(0, 0);

	float sum = weights[0];
	for (UINT i = 0; i < blurCount / 2; i++ )
	{
		float temp = GetGaussianFuntion((float)(i + 1));

		weights[i * 2 + 1] = temp;
		weights[i * 2 + 2] = temp;
		sum += temp * 2;

		Vector2 temp2 = Vector2(x, y) * (i * 2 + 1.5f);
		offsets[i * 2 + 1] = temp2;
		offsets[i * 2 + 2] = -temp2;
	}

	for (UINT i = 0; i < blurCount; i++)
	{
		weights[i] /= sum;
	}
}

float BloomDemo::GetGaussianFuntion(float val)
{
	return (float)((1.0f / sqrt(2 * Math::PI * blurCount)) * exp(-(val * val) / (2 * blurCount * blurCount)));
}


