#include "stdafx.h"
#include "ModelEditor.h"
#include "Systems/ImSequencer.h"
#include "Converter.h"
#include "Utilities/Xml.h"
#include "Systems/imgui_internal.h"
#include "GizmoFunc.h"
#include <filesystem>

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

	playeButton = new Texture(L"PlayButton.png");
	pauseButton = new Texture(L"PauseButton.png");
	stopButton = new Texture(L"StopButton.png");
	Mesh();
}

void ModelEditor::Destroy()
{
	SafeDelete(shader)
		SafeDelete(shadow)

		SafeDelete(sky)

		SafeDelete(floor)
		SafeDelete(stone)

		SafeDelete(sphere)
		SafeDelete(grid)

		SafeDelete(particleSystem)

		SafeDelete(modelAnimator)

		for (auto& list : modelLists)
		{
			SafeDelete(list)
		}
	for (auto& texture : textures)
	{
		SafeDelete(texture)
	}
}

void ModelEditor::Update()
{
	//Imgui Set
	{
		MainMenu();
		Gizmo();
		Project();
		Hiarachy();
		Inspector();
		Animation();
	}
	//if (Mouse::Get()->DoubleClick(0))
	//{
	//	transformNum = sphere->GetPickedPosition();
	//	if (transformNum != UINT32_MAX)//?? ==
	//	{
	//		editorState = BONE_EDITOR_STATE;
	//	}
	//}
	//static string str; 
	//str = to_string(picked.x) + ", " + to_string(picked.y) + " , " + to_string(picked.z);
	//Gui::Get()->RenderText(Vector2(10, 60), Color(1, 0, 0, 1), "Raycast : " + str);


	sky->Update();

	grid->Update();
	sphere->Update();


	if (particleSystem != NULL)
	{
		/*	particleSystem->Add(P);
			particleSystem->Update();*/
	}

	if (modelLists.size() != 0 && currentModelID != -1)
	{
		modelLists[currentModelID]->Update();
		
		BoneView();
		BoneSphereUpdate();
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


	Pass(7);
	floor->Render();
	grid->Render();


	if (modelLists.size() != 0 && currentModelID != -1)
	{
		modelLists[currentModelID]->Render();
	}

	//32_model.fx
	{
		stone->Render();
		sphere->Pass(3);
		sphere->Render();
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
		stone->DiffuseMap("Blue.png");
		stone->SpecularMap("Blue.png");
		stone->NormalMap("Blue.png");
		stone->Specular(0.3f, 0.3f, 0.3f, 30.0f);
		stone->Emissive(0.2f, 0.2f, 0.2f, 0.7f);
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
				if (ImGui::MenuItem(".png"))
				{
					Path::OpenFileDialog
					(
						openPngFile,
						L"Mesh_file\0*.png",
						L"../../_Textures",
						bind(&ModelEditor::OpenPngFile, this, placeholders::_1),
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
			DragAndDropTreeNode("Meshes");
		}
	}
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (openPngFile != L"")
		{

			DragAndDropTreeNode("Materials");

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
					if(bCount == true)
					{
						for (int i = 0; i < MAX_MODEL_TRANSFORMS; i++)
						{
							D3DXMatrixIdentity(&sphere->GetTransform(i)->World());
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
	bOpen = ImGui::Begin("Inspector", &bOpen, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
	ImGui::Separator();
	ImGui::Separator();

	if (-1 != currentModelID)
	{
		ImGui::Columns(4, "grid", true);
		ImGui::TextColored(ImVec4(0.3f, 0.6f, 0.9f, 1.0f), "Mat_Name", ImVec2(80, 80));
		ImGui::NextColumn();
		ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Diff", ImVec2(80, 80));
		ImGui::NextColumn();
		ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.0f, 1.0f), "Spec", ImVec2(80, 80));
		ImGui::NextColumn();
		ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.9f, 1.0f), "Normal", ImVec2(80, 80));
		ImGui::NextColumn();
		for (UINT i = 0; i < 4; i++)
		{
			ImGui::SetColumnWidth(i, 88);
		}
		ImGui::Dummy(ImVec2(32, 32));

		for (int i = 0; i < modelLists[currentModelID]->GetModel()->MeshCount(); i++)
		{
			ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.9f, 1.0f), String::ToString(modelLists[currentModelID]->GetModel()->Materials()[i]->Name()).c_str());
			ImVec2 tempV = ImGui::CalcTextSize("S");
			ImGui::Dummy(ImVec2(64, 68 - tempV.y));
		}
		ImGui::NextColumn();
		int frame_padding = -1;     // -1 = uses default padding
		for (int i = 0; i < modelLists[currentModelID]->GetModel()->MeshCount(); i++)
		{
			if (ImGui::ImageButton(*modelLists[currentModelID]->GetModel()->MeshByIndex(i)->GetMaterial()->DiffuseMap()
				, ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)))
			{

			}
			int index = DragAndDropReceiver("Png");
			if (index >= 0)
			{
				*modelLists[currentModelID]->GetModel()->MeshByIndex(i)->GetMaterial()->DiffuseMap() = *textures[index];
				*modelLists[currentModelID]->GetModel()->MaterialByIndex(i)->DiffuseMap() = *textures[index];
			}
		}
		ImGui::NextColumn();
		for (int i = 0; i < modelLists[currentModelID]->GetModel()->MeshCount(); i++)
		{

			if (ImGui::ImageButton(*modelLists[currentModelID]->GetModel()->MeshByIndex(i)->GetMaterial()->SpecularMap()
				, ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)))
			{

			}
			int index = DragAndDropReceiver("Png");
			if (index >= 0)
			{
				*modelLists[currentModelID]->GetModel()->MeshByIndex(i)->GetMaterial()->SpecularMap() = *textures[index];
				*modelLists[currentModelID]->GetModel()->MaterialByIndex(i)->SpecularMap() = *textures[index];
			}
		}
		ImGui::NextColumn();
		for (int i = 0; i < modelLists[currentModelID]->GetModel()->MeshCount(); i++)
		{

			if (ImGui::ImageButton(*modelLists[currentModelID]->GetModel()->MeshByIndex(i)->GetMaterial()->NormalMap()
				, ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)))
			{

			}
			int index = DragAndDropReceiver("Png");
			if (index >= 0)
			{
				*modelLists[currentModelID]->GetModel()->MeshByIndex(i)->GetMaterial()->NormalMap() = *textures[index];
				*modelLists[currentModelID]->GetModel()->MaterialByIndex(i)->NormalMap() = *textures[index];
			}
		}

	}



	//
	ImGui::End();
}
void ModelEditor::Gizmo()
{
	ImGuizmo::BeginFrame();

	ImGui::Begin("Editor");
	ImGui::Text("Camera");


	ImGui::Separator();
	ImGuizmo::SetID(0);

	if (modelLists.size() != 0 && currentModelID != -1 && editorState == MESH_EDITOR_STATE)
	{
		EditTransform(Context::Get()->View(), Context::Get()->Projection(), &modelLists[currentModelID]->GetTransform(0)->World()[0], lastUsing == 0);
		modelLists[currentModelID]->UpdateTransforms();
	}
	ImGui::End();
}
void ModelEditor::Animation()
{
	//Todo 
	bool bOpen = true;

	const char* str[eClipNameMaxNum] = { 0 };
	static const char* current_item = NULL;
	static int currentClipNum = -1;


	ImGui::Begin("Animation", &bOpen, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
	if (currentModelID != -1)
	{
		for (int i = 0; i < clipLists[currentModelID].size(); ++i)
		{
			str[i] = clipLists[currentModelID][i].c_str();
		}

		ImGui::PushItemWidth(150);
		if (ImGui::BeginCombo("Clips", current_item))
		{

			for (int n = 0; n < clipLists[currentModelID].size(); n++)
			{
				bool is_selected = (current_item == str[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(str[n], is_selected))
				{
					current_item = str[n];
					currentClipNum = n;
					//

					//
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}
	}

	ImGui::SameLine();
	ImGui::Indent(200);
	if (ImGui::ImageButton(*playeButton
		, ImVec2(20, 16), ImVec2(0.0, 0), ImVec2(1, 1), 2, ImVec4(1.0f, 1.0f, 1.0f, 0.0f)))
	{
		if (currentClipNum != -1)
		{
			editorState = ANIM_EDITOR_STATE;
			modelLists[currentModelID]->Pass(2);
			modelLists[currentModelID]->PlayClip(0, currentClipNum, 1.0f, 1.0f);
			tempDebugvalue = true;
		}
	}

	ImGui::SameLine();
	if (ImGui::ImageButton(*pauseButton
		, ImVec2(20, 16), ImVec2(0.0, 0), ImVec2(1, 1), 2, ImVec4(1.0f, 1.0f, 1.0f, 0.0f)))
	{

	}
	ImGui::SameLine();
	if (ImGui::ImageButton(*stopButton
		, ImVec2(20, 16), ImVec2(0.0, 0), ImVec2(1, 1), 2, ImVec4(1.0f, 1.0f, 1.0f, 0.0f)))
	{
		tempDebugvalue = false;
		modelLists[currentModelID]->Pass(1);
		current_item = NULL;
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
	else if (ext == L"PNG")
	{
		OpenPngFile(fileDirectory + L"/" + fileName);
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
	wstring fileFullDirectory = Path::GetDirectoryName(file);
	openFile = Path::GetFileNameWithoutExtension(file);
	projectMeshNames.push_back(String::ToString(openFile));

	ModelAnimMultiBone * modelRender = new ModelAnimMultiBone(modelShader);
	modelRender->ReadMaterial(fileDirectory + L"/" + fileName);
	modelRender->ReadMesh(fileDirectory + L"/" + fileName);
	//해당 fileDirectory 폴더 있는 확장자가 .clip 파일을 다 불러와서 클립을 읽는다.
	std::string path("../../_Models/" + String::ToString(fileFullDirectory));
	std::string ext(".clip");
	for (auto& p : std::experimental::filesystem::recursive_directory_iterator(path))
	{
		if (p.path().extension() == ext)
		{
			clipNames.push_back(String::ToString(p.path().filename().c_str()));
		}
	}
	for (auto name : clipNames)
	{
		modelRender->ReadClip(fileDirectory + L"/" + Path::GetFileNameWithoutExtension(String::ToWString(name)));
	}
	//
	Transform* attachTransform = modelRender->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);

	modelRender->UpdateTransforms();
	modelRender->Pass(1);
	modelLists.push_back(modelRender);
}

void ModelEditor::OpenMeshFile(wstring file)
{
	wstring fileDirectory = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);
	wstring fileFullDirectory = Path::GetDirectoryName(file);

	openFile = Path::GetFileNameWithoutExtension(file);
	projectMeshNames.push_back(String::ToString(openFile));
	ModelAnimMultiBone* modelRender = new ModelAnimMultiBone(modelShader);
	modelRender->ReadMaterial(fileDirectory + L"/" + fileName);
	modelRender->ReadMesh(fileDirectory + L"/" + fileName);

	//해당 fileDirectory 폴더 있는 확장자가 .clip 파일을 다 불러와서 클립을 읽는다.
	std::string path("../../_Models/" + String::ToString(fileFullDirectory));
	std::string ext(".clip");

	if (!clipNames.empty())
	{
		clipNames.clear();
	}

	for (auto& p : std::experimental::filesystem::recursive_directory_iterator(path))
	{
		if (p.path().extension() == ext)
		{
			clipNames.push_back(String::ToString(p.path().filename().c_str()));
		}
	}

	clipLists.push_back(clipNames);

	for (auto name : clipNames)
	{
		modelRender->ReadClip(fileDirectory + L"/" + Path::GetFileNameWithoutExtension(String::ToWString(name)));
	}
	//
	Transform* attachTransform = modelRender->AddTransform();
	attachTransform->Position(-10, 0, -10);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);

	modelRender->UpdateTransforms();
	modelRender->Pass(1);

	modelLists.push_back(modelRender);
}

void ModelEditor::OpenPngFile(wstring file)
{
	openPngFile = Path::GetFileName(file);
	projectPngFileNames.push_back(String::ToString(openPngFile));
	Texture* texture = new Texture(openPngFile);
	textures.push_back(texture);
}

void ModelEditor::BoneView()
{
	if (bCount == false)
	{
		Transform* transform = NULL;
		for (UINT i = 0; i < MAX_MODEL_TRANSFORMS; i++)
		{
			transform = sphere->AddTransform();
			transform->Scale(1.0f, 1.0f, 1.0f);
		//	D3DXMatrixInverse(&transform->World(),NULL, &transform->World());
		}
		bCount = true;
	}
	sphere->UpdateTransforms();
}

void ModelEditor::BoneSphereUpdate()
{
	Matrix W = modelLists[currentModelID]->GetTransform(0)->World();
	if (tempDebugvalue == true)
	{
		for (UINT i = 0; i < modelLists[currentModelID]->GetModel()->BoneCount(); i++)
		{
			Matrix m = modelLists[currentModelID]->GetBoneMatrix(i);
			sphere->GetTransform(i)->World() = modelLists[currentModelID]->GetModel()->Bones()[i]->Transform() * m * W;
			sphere->GetTransform(i)->World()._11 = 0.4f;
			sphere->GetTransform(i)->World()._22 = 0.4f;
			sphere->GetTransform(i)->World()._33 = 0.4f;
		}
		
		sphere->UpdateTransforms();
		modelLists[currentModelID]->UpdateTransforms();
	} 
	else
	{
		for (UINT i = 0; i < modelLists[currentModelID]->GetModel()->BoneCount(); i++)
		{
			sphere->GetTransform(i)->World() = modelLists[currentModelID]->GetModel()->Bones()[i]->Transform() * W;
			sphere->GetTransform(i)->World()._11 = 0.4f;
			sphere->GetTransform(i)->World()._22 = 0.4f;
			sphere->GetTransform(i)->World()._33 = 0.4f;
		}
		sphere->UpdateTransforms();
		modelLists[currentModelID]->UpdateTransforms();
	}
}

void ModelEditor::DragAndDropTreeNode(const char* label)
{
	if ("Meshes" == label)
	{
		for (int n = 0; n < projectMeshNames.size(); n++)
		{
			ImGui::PushID(n);
			if (ImGui::TreeNode(projectMeshNames[n].c_str()))
			{
				ImGui::TreePop();
			}
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
	else if ("Materials" == label)
	{
		for (int n = 0; n < projectPngFileNames.size(); n++)
		{
			ImGui::PushID(n);
			if (ImGui::TreeNode(projectPngFileNames[n].c_str()))
			{
				ImGui::TreePop();
			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				ImGui::SetDragDropPayload("Png", &n, sizeof(int));    // Set payload to carry the index of our item (could be anything)

				//ImGui::Text("%s", projectPngFileNames[n].c_str());
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("Png"))
				{
					IM_ASSERT(payload->DataSize == sizeof(int));
					int payload_n = *(const int*)payload->Data;
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopID();
		}
	}

}

int ModelEditor::DragAndDropReceiver(const char* name)
{
	int indexNum = -1;
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload(name))
		{
			IM_ASSERT(payload->DataSize == sizeof(int));
			int payload_n = *(const int*)payload->Data;
			indexNum = payload_n;
			assert(indexNum >= 0, "accepted value of nagtive");
		}
		ImGui::EndDragDropTarget();
	}
	return indexNum;
}
