#include "stdafx.h"
#include "ModelEditor.h"
#include "Converter.h"
#include "Utilities/Xml.h"

void ModelEditor::Initialize()
{
	importer = new Assimp::Importer();

	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 25, -50);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 5);

	shader = new Shader(L"57_ParticleViewer.fxo");
	modelShader = new Shader(L"32_Model.fxo");
	shadow = new Shadow(shader, Vector3(0, 0, 0), 65);

	sky = new Sky(shader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.5f);

	Mesh();
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
	SafeDelete(importer);
	SafeDelete(modelAnimator)
}

void ModelEditor::Update()
{
	//Imgui Set
	{
		MainMenu();
		Project();
		Hiarachy();
		Inspector();
	}

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

	if (modelAnimator != NULL)
	{
		modelAnimator->Update();
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

	if (modelAnimator != NULL)
	{
		modelAnimator->Render();
	}
}


////////////////////////////////////

void ModelEditor::Mesh()
{
	//Create Material
	{
		//바닥
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->SpecularMap("Floor_Specular.png");
		floor->NormalMap("Floor_Normal.png");
		floor->Specular(1, 1, 1, 15);
		floor->Emissive(0.2f, 0.2f, 0.2f, 0.3f);

		//구
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


///////////////////////////////////////
//ImGui
void ModelEditor::MainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
			D3DDesc desc = D3D::GetDesc();

			if (ImGui::BeginMenu("Open"))
			{ 
				if (ImGui::MenuItem(".fbx"))
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
				if (ImGui::MenuItem(".mesh"))
				{
					Path::OpenFileDialog
					(
						openFile,
						L"Mesh_file\0*.mesh",
						L"../../_Models",
						bind(&ModelEditor::OpenFile, this, placeholders::_1),
						desc.Handle
					);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Save"))
			{
			
				if (ImGui::MenuItem(".mesh"))
				{
					Path::SaveFileDialog
					(
						openFile,
						L".mesh\0*.mesh",
						L"../../_Models",
						bind(&ModelEditor::WriteMeshFile, this, placeholders::_1),
						desc.Handle
					);
				}
				if (ImGui::MenuItem(".material"))
				{
					Path::SaveFileDialog
					(
						openFile,
						L".material\0*.material",
						L"../../_Textures",
						bind(&ModelEditor::WriteMaterialFile, this, placeholders::_1),
						desc.Handle
					);
				}
				ImGui::EndMenu();
			}	
		ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();
}

void ModelEditor::Project()
{
	
	bool bOpen = true;
	bOpen = ImGui::Begin("Project", &bOpen);
	//ImGui::SetWindowPos(ImVec2(width - windowWidth, 0));
	//ImGui::SetWindowSize(ImVec2(windowWidth, height));
	ImGui::TextColored(ImVec4(0.1f, 0.1f, 0.9f, 1.0f), "Models");
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Meshes"), ImGuiTreeNodeFlags_DefaultOpen)
	{

	}
	if (ImGui::CollapsingHeader("Materials"), ImGuiTreeNodeFlags_DefaultOpen)
	{

	}
	if (ImGui::CollapsingHeader("Behavior Trees"), ImGuiTreeNodeFlags_DefaultOpen)
	{

	}
	ImGui::End();

}

void ModelEditor::Hiarachy()
{
	bool bOpen = true;
	bOpen = ImGui::Begin("Hiarachy", &bOpen);
	//ImGui::SetWindowPos(ImVec2(width - windowWidth, 0));
	//ImGui::SetWindowSize(ImVec2(windowWidth, height));
	ImGui::TextColored(ImVec4(0.1f, 0.1f, 0.9f, 1.0f), "Models");
	ImGui::Separator();
	//if (ImGui::CollapsingHeader("Meshes"), ImGuiTreeNodeFlags_DefaultOpen)
	//{

	//}
	ImGui::End();
}

void ModelEditor::Inspector()
{
	bool bOpen = true;
	bOpen = ImGui::Begin("Inspector", &bOpen);
	ImGui::Separator();
	ImGui::Separator();
	ImGui::End();
}
//ImGui
///////////////////////////////////////////

void ModelEditor::Pass(UINT meshPass)
{
	for (MeshRender* mesh : meshes)
		mesh->Pass(meshPass);
}

void ModelEditor::WriteMeshFile(wstring file)
{
	if (modelAnimator != NULL)
	{
		modelAnimator->GetModel()->WriteMeshData(file,false);
	}
}

void ModelEditor::WriteMaterialFile(wstring file)
{
	if (modelAnimator != NULL)
	{
		modelAnimator->GetModel()->WriteMaterial(file, false);
	}
}

void ModelEditor::OpenFile(wstring file)
{
	//파일 확장자가 fbx냐 .mesh 에따라 따르게 만듬.
	//fbx면 일단 기본저장된 mesh정보를 읽고 쓴정보를 다시 불러와 모델을만듬
	//.mesh이면 mesh,matrial 정보를찾아서 불러옴 \
	//또다른 포맷 ex).mod 다른 바이너리 파일 -- 나중에  \

	wstring ext = Path::GetExtension(file);
	wstring fileDirectory = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);
	transform(ext.begin(), ext.end(), ext.begin(), toupper);
	if (ext == L"FBX")
	{
		//어떻게 파일명 ? 
		OpenFbxFile(fileDirectory + L"/" + fileName); // ex)Archer/Archer
	}
	else if (ext == L"MESH")
	{
		OpenMeshFile(fileDirectory + L"/" +fileName);
	}
}

void ModelEditor::OpenFbxFile(wstring file)
{
	Converter* conv = new Converter();
	conv->ReadFile(file+L".fbx");
	conv->ExportMaterial(file, false);
	conv->ExportMesh(file,false);
	SafeDelete(conv);

	wstring fileDirectory = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);
	modelAnimator = new ModelRender(modelShader);
	modelAnimator->ReadMaterial(fileDirectory +L"/"+fileName);
	modelAnimator->ReadMesh(fileDirectory + L"/"+ fileName);
	
	Transform* attachTransform = modelAnimator->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);
	

	modelAnimator->UpdateTransforms();
	modelAnimator->Pass(1);
}

void ModelEditor::OpenMeshFile(wstring file)
{
	wstring fileDirectory = Path::GetLastDirectoryName(file);

	modelAnimator = new ModelRender(modelShader);
	modelAnimator->ReadMaterial(fileDirectory + L"/Mesh");
	modelAnimator->ReadMesh(fileDirectory + L"/Mesh");

	Transform * attachTransform = modelAnimator->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);

	modelAnimator->UpdateTransforms();
	modelAnimator->Pass(1);
}
