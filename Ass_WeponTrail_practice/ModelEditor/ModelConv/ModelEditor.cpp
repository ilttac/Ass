#include "stdafx.h"
#include "ModelEditor.h"
#include "Systems/ImSequencer.h"
#include "Converter.h"
#include "Utilities/Xml.h"
#include "Systems/imgui_internal.h"
#include "GizmoFunc.h"

void ModelEditor::Initialize()
{
	// sequence with default values
	mySequence.mFrameMin = -100;
	mySequence.mFrameMax = 1000;
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 0, 10, 30, false });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 1, 20, 30, true });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 3, 12, 60, false });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 2, 61, 90, false });
	mySequence.myItems.push_back(MySequence::MySequenceItem{ 4, 90, 99, false });

	importer = new Assimp::Importer();

	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 25, -50);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 5);

	D3DXMatrixIdentity(&matrixIdentity);

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

		for (auto& list : modelLists)
		{
			SafeDelete(list);
		}

}

void ModelEditor::Update()
{
	///////////////


	//Imgui Set
	{
		MainMenu();
		Gizmo();
		Project();
		Hiarachy();
		Inspector();
	}
	if (Mouse::Get()->DoubleClick(0))
	{
		transformNum = sphere->GetPickedPosition();
		if (transformNum != UINT32_MAX)//?? ==
		{
			editorState = BONE_EDITOR_STATE;
		}
	}
	//static string str; 
	//str = to_string(picked.x) + ", " + to_string(picked.y) + " , " + to_string(picked.z);
	//Gui::Get()->RenderText(Vector2(10, 60), Color(1, 0, 0, 1), "Raycast : " + str);


	sky->Update();

	grid->Update();
	sphere->Update();

	//Vector3 P;
	//sphere->GetTransform(0)->Position(&P);
	//float moveSpeed = 30.0f;

	//if (Mouse::Get()->Press(1) == false)
	//{
	//	const Vector3& F = Context::Get()->GetCamera()->Foward();
	//	const Vector3& R = Context::Get()->GetCamera()->Right();
	//	const Vector3& U = Context::Get()->GetCamera()->Up();
	//	if (Keyboard::Get()->Press('W'))
	//		P += Vector3(F.x, 0, F.z) * moveSpeed * Time::Delta();
	//	else if (Keyboard::Get()->Press('S'))
	//		P += Vector3(-F.x, 0, -F.z) * moveSpeed * Time::Delta();

	//	if (Keyboard::Get()->Press('A'))
	//		P += -R * moveSpeed * Time::Delta();
	//	else if (Keyboard::Get()->Press('D'))
	//		P += R * moveSpeed * Time::Delta();

	//	if (Keyboard::Get()->Press('E'))
	//		P += U * moveSpeed * Time::Delta();
	//	else if (Keyboard::Get()->Press('Q'))
	//		P += -U * moveSpeed * Time::Delta();
	//}
	//sphere->GetTransform(0)->Position(P);
	//sphere->UpdateTransforms();

	if (particleSystem != NULL)
	{
		/*	particleSystem->Add(P);
			particleSystem->Update();*/
	}

	if (modelLists.size() != 0 && currentModelID != -1)
	{
		modelLists[currentModelID]->Update();
		modelBones = modelLists[currentModelID]->GetModel()->Bones();
		BoneView();
	}
}

void ModelEditor::PreRender()
{
	sky->PreRender();
	//Depth
	{
		shadow->Set();
		Pass(0);
		//sphere->Render();
	}

}

void ModelEditor::Render()
{
	sky->Pass(4, 5, 6);
	sky->Render();

	//32_model.fx
	{
		stone->Render();
		sphere->Pass(1);
		sphere->Render();
	}

	Pass(7);
	floor->Render();
	grid->Render();


	if (modelLists.size() != 0 && currentModelID != -1)
	{
		modelLists[currentModelID]->Render();
	}
	if (particleSystem != NULL)
	{
		particleSystem->Render();
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
		stone = new Material(modelShader);
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

		//MeshSphere* meshSphere = new MeshSphere(0.5f, 20, 20);
		//meshSphere->Create();
		sphere = new MeshRender(modelShader, new MeshSphere(0.5f, 20, 20));

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
		if (ImGui::BeginMenu("File"))
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

	if (ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (openFile != L"")
		{
			DragAndDropTreeNode("Project");
		}
	}
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (openFile != L"")
		{
			if (ImGui::TreeNode(String::ToString(openFile).c_str()))
			{

				ImGui::TreePop();
			}
		}
	}
	if (ImGui::CollapsingHeader("Behavior Trees", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (openFile != L"")
		{
			if (ImGui::TreeNode(String::ToString(openFile).c_str()))
			{

				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

void ModelEditor::Hiarachy()
{
	bool bOpen = true;
	bOpen = ImGui::Begin("Hiarachy", &bOpen);

	static ImGuiTextFilter filter;
	filter.Draw("Objects", 150.f);
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
		{
			IM_ASSERT(payload->DataSize == sizeof(int));
			int payload_n = *(const int*)payload->Data;
			hiarachyName.push_back(projectMeshNames[payload_n]);
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::Separator();

	if (openFile != L"")
	{
		if (hiarachyName.size() != 0)
		{
			static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
			int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
			for (int i = 0; i < hiarachyName.size(); i++)
			{
				// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
				const bool is_selected = (selection_mask & (1 << i)) != 0;
				if (is_selected)
					node_flags |= ImGuiTreeNodeFlags_Selected;
				bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, (hiarachyName[i]).c_str());
				if (ImGui::IsItemClicked())
				{
					node_clicked = i;
					int index = -1;

					for (int j = 0; j < projectMeshNames.size(); j++)
					{
						if (hiarachyName[i] == projectMeshNames[j])
						{
							index = j;
							break;
						}
					}

					currentModelID = index;
				}

				if (node_open)
				{
					ImGui::TreePop();
				}
			}
		}
	}

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
void ModelEditor::Gizmo()
{
	static const float identityMatrix[16] =
	{ 1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuizmo::BeginFrame();

	ImGui::Begin("Editor");
	ImGui::Text("Camera");
	bool viewDirty = false;
	if (ImGui::RadioButton("Perspective", isPerspective)) isPerspective = true;
	ImGui::SameLine();
	if (ImGui::RadioButton("Orthographic", !isPerspective)) isPerspective = false;
	if (isPerspective)
	{
		ImGui::SliderFloat("Fov", &fov, 20.f, 110.f);
	}
	else
	{
		ImGui::SliderFloat("Ortho width", &viewWidth, 1, 20);
	}
	viewDirty |= ImGui::SliderFloat("Distance", &camDistance, 1.f, 10.f);
	ImGui::SliderInt("Gizmo count", &gizmoCount, 1, 4);

	ImGui::Separator();
	for (int matId = 0; matId < gizmoCount; matId++)
	{
		ImGuizmo::SetID(matId);
		if (modelLists.size() != 0 && currentModelID != -1 && editorState == MESH_EDITOR_STATE)
		{
			EditTransform(Context::Get()->View(), Context::Get()->Projection(), &modelLists[currentModelID]->GetTransform(0)->World()[matId], lastUsing == matId);
			modelLists[currentModelID]->UpdateTransforms();
		}
		else if (modelLists.size() != 0 && currentModelID != -1 && editorState == BONE_EDITOR_STATE && transformNum != UINT32_MAX)
		{
			EditTransform(Context::Get()->View(), Context::Get()->Projection(), &sphere->GetTransform(transformNum)->World()[matId], lastUsing == matId);
			sphere->UpdateTransforms();
			//modelLists[currentModelID]->GetTransform(0)->World() = sphere->GetTransform(transformNum)->World();
			modelLists[currentModelID]->GetModel()->Bones()[transformNum]->Transform() = sphere->MeshTransformWorld(transformNum);
			modelLists[currentModelID]->UpdateTransform(currentModelID,transformNum,*sphere->GetTransform(transformNum));
			modelLists[currentModelID]->UpdateTransforms();
		}
		if (ImGuizmo::IsUsing())
		{
			lastUsing = matId;
		}
	}
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
	if (modelLists.size() != 0 && currentModelID != -1)
	{
		modelLists[currentModelID]->GetModel()->WriteMeshData(file, false);
	}
}

void ModelEditor::WriteMaterialFile(wstring file)
{
	if (modelLists.size() != 0 && currentModelID != -1)
	{
		modelLists[currentModelID]->GetModel()->WriteMaterial(file, false);
	}
}

void ModelEditor::OpenFile(wstring file)
{
	//파일 확장자가 fbx냐 .mesh 에따라 따르게 만듬.
	//fbx면 일단 기본저장된 mesh정보를 읽고 쓴정보를 다시 불러와 모델을만듬
	//.mesh이면 mesh,matrial 정보를찾아서 불러옴 \
	//또다른 포맷 ex).mod 다른 바이너리 파일 -- 나중에  

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
		OpenMeshFile(fileDirectory + L"/" + fileName);
	}
}

void ModelEditor::OpenFbxFile(wstring file)
{
	Converter* conv = new Converter();
	conv->ReadFile(file + L".fbx");
	conv->ExportMaterial(file, false);
	conv->ExportMesh(file, false);
	SafeDelete(conv);

	wstring fileDirectory = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);
	openFile = Path::GetFileNameWithoutExtension(file);
	projectMeshNames.push_back(String::ToString(openFile));

	ModelRender * modelRender = new ModelRender(modelShader);
	modelRender->ReadMaterial(fileDirectory + L"/" + fileName);
	modelRender->ReadMesh(fileDirectory + L"/" + fileName);

	Transform * attachTransform = modelRender->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);

	modelRender->UpdateTransforms();
	modelRender->Pass(1);
	modelLists.push_back(modelRender);
}

void ModelEditor::OpenMeshFile(wstring file)
{
	wstring fileDirectory = Path::GetLastDirectoryName(file);
	openFile = Path::GetFileNameWithoutExtension(file);
	projectMeshNames.push_back(String::ToString(openFile));
	ModelRender* modelRender = new ModelRender(modelShader);
	modelRender->ReadMaterial(fileDirectory + L"/Mesh");
	modelRender->ReadMesh(fileDirectory + L"/Mesh");

	Transform * attachTransform = modelRender->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);

	modelRender->UpdateTransforms();
	modelRender->Pass(1);
	modelLists.push_back(modelRender);
}

void ModelEditor::BoneView()
{
	static int count = 0;
	if (count == 0)
	{
		vector<Matrix*> sphereTransform;
		vector<Matrix> modelBoneTransform;
		Matrix W = modelLists[currentModelID]->GetTransform(0)->World();
		Transform* transform = NULL;
		sphereTransform.reserve(500);
		for (UINT i = 0; i < (modelLists[currentModelID]->GetModel()->Bones().size()); i++)
		{
			transform = sphere->AddTransform();
			transform->Scale(1.0f, 1.0f, 1.0f);
			modelBoneTransform.push_back(modelBones[i]->Transform());
		}
		for (UINT i = 0; i < (modelLists[currentModelID]->GetModel()->Bones().size()); i++)
		{
			sphereTransform.push_back(&sphere->GetTransform(i)->World());
			D3DXMatrixInverse(sphereTransform[i], NULL, sphereTransform[i]);
			*sphereTransform[i] *= modelBoneTransform[i] * W;
			sphereTransform[i]->_11 = 0.2f;
			sphereTransform[i]->_22 = 0.2f;
			sphereTransform[i]->_33 = 0.2f;
		}
		count = 1;
	}
	sphere->UpdateTransforms();
}

void ModelEditor::DragAndDropTreeNode(const char* label)
{

	for (int n = 0; n < projectMeshNames.size(); n++)
	{
		ImGui::PushID(n);
		if (ImGui::TreeNode(projectMeshNames[n].c_str()))
		{
			ImGui::TreePop();
		}

		// Our buttons are both drag sources and drag targets here!
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));    // Set payload to carry the index of our item (could be anything)

			ImGui::Text("%s", projectMeshNames[n].c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
			{
				IM_ASSERT(payload->DataSize == sizeof(int));
				int payload_n = *(const int*)payload->Data;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopID();
	}
}
