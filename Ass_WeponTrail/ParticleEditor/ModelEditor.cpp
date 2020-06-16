#include "stdafx.h"
#include "ModelEditor.h"
#include "Utilities/Xml.h"

void ModelEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 25, -50);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 5);

	shader = new Shader(L"57_ParticleViewer.fxo");
	modelShader = new Shader(L"33_Animation.fxo");
	shadow = new Shadow(shader, Vector3(0, 0, 0), 65);

	sky = new Sky(shader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.5f);

	Mesh();

	UpdateParticleList();
	UpdateTextureList();
	UpdateModelAnimList();
}

void ModelEditor::Destroy()
{
	SafeDelete(shader);
	SafeDelete(shadow);

	SafeDelete(sky);

	SafeDelete(floor);
	SafeDelete(stone);

	SafeDelete(sphere);
	SafeDelete(grid);

	SafeDelete(particleSystem);
}

void ModelEditor::Update()
{
	OnGUI();

	sky->Update();

	grid->Update();
	sphere->Update();

	Vector3 P;
	sphere->GetTransform(0)->Position(&P);
	float moveSpeed = 30.0f;

	if (Mouse::Get()->Press(1) == false)
	{
		const Vector3& F = Context::Get()->GetCamera()->Foward();
		const Vector3& R = Context::Get()->GetCamera()->Right();
		const Vector3& U = Context::Get()->GetCamera()->Up();
		if (Keyboard::Get()->Press('W'))
			P += Vector3(F.x, 0, F.z) * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('S'))
			P += Vector3(-F.x, 0, -F.z) * moveSpeed * Time::Delta();

		if (Keyboard::Get()->Press('A'))
			P += -R * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('D'))
			P += R * moveSpeed * Time::Delta();

		if (Keyboard::Get()->Press('E'))
			P += U * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('Q'))
			P += -U * moveSpeed * Time::Delta();
	}
	sphere->GetTransform(0)->Position(P);
	sphere->UpdateTransforms();

	if (particleSystem != NULL)
	{
		particleSystem->Add(P);
		particleSystem->Update();
	}
}

void ModelEditor::PreRender()
{
	sky->PreRender();

	//Depth
	{
		shadow->Set();
		Pass(0);
		sphere->Render();
	}
}

void ModelEditor::Render()
{
	sky->Pass(4, 5, 6);
	sky->Render();

	Pass(7);
	stone->Render();
	sphere->Render();

	floor->Render();
	grid->Render();

	if (particleSystem != NULL)
	{
		particleSystem->Render();
	}
}

void ModelEditor::Mesh()
{
	//Create Material
	{
		//�ٴ�
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->SpecularMap("Floor_Specular.png");
		floor->NormalMap("Floor_Normal.png");
		floor->Specular(1, 1, 1, 15);
		floor->Emissive(0.2f, 0.2f, 0.2f, 0.3f);

		//��
		stone = new Material(shader);
		stone->DiffuseMap("Bricks.png");
		stone->SpecularMap("Bricks_Specular.png");
		stone->NormalMap("Bricks_Normal.png");
		stone->Specular(0.3f, 0.3f, 0.3f, 20.0f);
		stone->Emissive(0.2f, 0.2f, 0.2f, 0.3f);
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		grid = new MeshRender(shader, new MeshGrid(15, 15));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 1, 20);

		sphere = new MeshRender(shader, new MeshSphere(0.5f, 20, 20));
		transform = sphere->AddTransform();
		transform->Position(0, 5, 0);
		transform->Scale(5, 5, 5);
	}

	sphere->UpdateTransforms();
	grid->UpdateTransforms();

	meshes.push_back(sphere);
	meshes.push_back(grid);
}

void ModelEditor::Pass(UINT meshPass)
{
	for (MeshRender* mesh : meshes)
		mesh->Pass(meshPass);
}

void ModelEditor::UpdateParticleList()
{

}

void ModelEditor::UpdateTextureList()
{

}

void ModelEditor::UpdateModelAnimList()
{
}

void ModelEditor::OnGUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
			D3DDesc desc = D3D::GetDesc();

			if (ImGui::MenuItem("Open", "CTRL+O"))
			{
				Path::OpenFileDialog
				(
					openFile,
					L"fbx_File\0*.fbx",
					L"../../_Assets",
					bind(&ModelEditor::OpenFile, this, placeholders::_1),
					desc.Handle
				);
			}
			else if (ImGui::MenuItem("Save", "CTRL+S")) 
			{
				Path::SaveFileDialog
				(
					saveFile,
					L"Mesh_file\0*.mesh",
					L"../../_Models",
					bind(&ModelEditor::WrtieFile, this, placeholders::_1),
					desc.Handle
				);
			}	
		ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();
}

void ModelEditor::OnGUI_LIst()
{
	if (openFile != L"")
	{
		openFile = Path::GetFileNameWithoutExtension(openFile);
	}
}

void ModelEditor::OnGUI_Settings()
{
	
}

void ModelEditor::OnGuI_Write()
{
	
}

void ModelEditor::WrtieFile(wstring file)
{
	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Particle");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);


	Xml::XMLElement* node = NULL;

	node = document->NewElement("BlendState");
	node->SetText((int)particleSystem->GetData().Type);
	root->LinkEndChild(node);


	string textureFile = String::ToString(particleSystem->GetData().TextureFile);
	String::Replace(&textureFile, "Particles/", "");

	node = document->NewElement("Loop");
	node->SetText(particleSystem->GetData().bLoop);
	root->LinkEndChild(node);

	node = document->NewElement("TextureFile");
	node->SetText(textureFile.c_str());
	root->LinkEndChild(node);


	node = document->NewElement("MaxParticles");
	node->SetText(particleSystem->GetData().MaxParticles);
	root->LinkEndChild(node);


	node = document->NewElement("ReadyTime");
	node->SetText(particleSystem->GetData().Readytime);
	root->LinkEndChild(node);

	node = document->NewElement("ReadyRandomTime");
	node->SetText(particleSystem->GetData().ReadyRandomTime);
	root->LinkEndChild(node);

	node = document->NewElement("StartVelocity");
	node->SetText(particleSystem->GetData().StartVelocity);
	root->LinkEndChild(node);

	node = document->NewElement("EndVelocity");
	node->SetText(particleSystem->GetData().EndVelocity);
	root->LinkEndChild(node);


	node = document->NewElement("MinHorizontalVelocity");
	node->SetText(particleSystem->GetData().MinHorizontalVelocity);
	root->LinkEndChild(node);

	node = document->NewElement("MaxHorizontalVelocity");
	node->SetText(particleSystem->GetData().MaxHorizontalVelocity);
	root->LinkEndChild(node);

	node = document->NewElement("MinVerticalVelocity");
	node->SetText(particleSystem->GetData().MinVerticalVelocity);
	root->LinkEndChild(node);

	node = document->NewElement("MaxVerticalVelocity");
	node->SetText(particleSystem->GetData().MaxVerticalVelocity);
	root->LinkEndChild(node);


	node = document->NewElement("Gravity");
	node->SetAttribute("X", particleSystem->GetData().Gravity.x);
	node->SetAttribute("Y", particleSystem->GetData().Gravity.y);
	node->SetAttribute("Z", particleSystem->GetData().Gravity.z);
	root->LinkEndChild(node);


	node = document->NewElement("MinColor");
	node->SetAttribute("R", particleSystem->GetData().MinColor.r);
	node->SetAttribute("G", particleSystem->GetData().MinColor.g);
	node->SetAttribute("B", particleSystem->GetData().MinColor.b);
	node->SetAttribute("A", particleSystem->GetData().MinColor.a);
	root->LinkEndChild(node);

	node = document->NewElement("MaxColor");
	node->SetAttribute("R", particleSystem->GetData().MaxColor.r);
	node->SetAttribute("G", particleSystem->GetData().MaxColor.g);
	node->SetAttribute("B", particleSystem->GetData().MaxColor.b);
	node->SetAttribute("A", particleSystem->GetData().MaxColor.a);
	root->LinkEndChild(node);


	node = document->NewElement("MinRotateSpeed");
	node->SetText(particleSystem->GetData().MinRotateSpeed);
	root->LinkEndChild(node);

	node = document->NewElement("MaxRotateSpeed");
	node->SetText(particleSystem->GetData().MaxRotateSpeed);
	root->LinkEndChild(node);

	node = document->NewElement("MinStartSize");
	node->SetText((int)particleSystem->GetData().MinStartSize);
	root->LinkEndChild(node);

	node = document->NewElement("MaxStartSize");
	node->SetText((int)particleSystem->GetData().MaxStartSize);
	root->LinkEndChild(node);

	node = document->NewElement("MinEndSize");
	node->SetText((int)particleSystem->GetData().MinEndSize);
	root->LinkEndChild(node);

	node = document->NewElement("MaxEndSize");
	node->SetText((int)particleSystem->GetData().MaxEndSize);
	root->LinkEndChild(node);

	wstring folder = Path::GetDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);

	document->SaveFile(String::ToString(folder + fileName + L".xml").c_str());
	SafeDelete(document);

	UpdateParticleList();
}

void ModelEditor::OpenFile(wstring file)
{
	openFile = file;

	modelAnimator = new ModelAnimator(modelShader);	
	
}
