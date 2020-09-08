#include "stdafx.h"
#include "SceneEditor.h"
#include "Utilities/Xml.h"
#include "GizmoFunc.h"
#include "LevelSaveLoad.h"

#include "../Framework/Utilities/BinaryFile.h"
#include "../Framework/Environment/Sky/Dome.h"
#include "../Framework/Environment/Sky/Moon.h"
#include "../Framework/Environment/Sky/Cloud.h"

#include <filesystem>

void SceneEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	terrainShader = new Shader(L"23_TerrainSpatting.fxo");

	shader_57 = new Shader(L"57_ParticleViewer.fxo");
	shader_53 = new Shader(L"53_DefferedShadow.fxo");
	shader_56 = new Shader(L"56_Billboard.fxo");
	modelShader = new Shader(L"33_Animation.fxo");
	BillboardSet();
	gBuffer = new GBuffer(shader_53);
	//shadow = new Shadow(skyShader, Vector3(0, 0, 0), 65);
	sky = new Sky(shader_57);
	sky->ScatteringPass(3);
	//sky->RealTime(true, Math::PI - 1e-6f, 0.5f);
}

void SceneEditor::Destroy()
{
	SafeDelete(terrainShader);
	SafeDelete(shadow);

	SafeDelete(sky);

	SafeDelete(terrain);
}

void SceneEditor::Update()
{
	
	if (terrain != NULL)
	{
		terrain->Update();
	}
	sky->Update();
	if (billBoard != NULL && billBoardCurID != -1)
	{
		billBoard->Update();
	}
	if (!modelLists.empty())
	{
		for (auto* staticMesh : modelLists)
		{
			staticMesh->Update();
			staticMesh->UpdateTransforms();
		}
	}
	//play누르면 안보이게 해야함.
	MainMenu();
	Hiarachy();
	ViewModel();
	Inspector();
	if (node_clicked != -1)
	{
		GuizmoUpdate(node_clicked);
	}
}

void SceneEditor::PreRender()
{
	gBuffer->PackGBuffer();
	sky->PreRender();
	////Depth
	//{
	//	shadow->Set();
	//	//sphere->Render();
	//}
}

void SceneEditor::Render()
{
	
	sky->Pass(4, 5, 6);
	sky->Render();
	if (terrain != NULL)
	{
		terrain->Pass(0);
		terrain->Render();
	}

	if (billBoard != NULL && billBoardCurID != -1)
	{
		billBoard->Pass(0);
		billBoard->Render();
	}

	if (!modelLists.empty())
	{
		for (auto* staticMesh : modelLists)
		{
			staticMesh->Render();
		}
	}
	gBuffer->Render();
}

void SceneEditor::PostRender()
{
	sky->PostRender();
}

void SceneEditor::MainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			D3DDesc desc = D3D::GetDesc();

			if (ImGui::BeginMenu("Open"))
			{
				if (ImGui::MenuItem(".trn"))
				{
					Path::OpenFileDialog
					(
						openTerrainFile,
						L"Terrain_File\0*.trn",
						L"../../_Textures",
						bind(&SceneEditor::OpenTrnFile, this, placeholders::_1),
						desc.Handle
					);
				}
				if (ImGui::MenuItem(".png"))
				{
					Path::OpenFileDialog
					(
						openTerrainFile,
						L"Terrain_File\0*256.png",
						L"../../_Textures",
						bind(&SceneEditor::OpenHeightMap, this, placeholders::_1),
						desc.Handle
					);
				}
				if (ImGui::MenuItem(".level"))
				{
					Path::OpenFileDialog
					(
						loadLevelFile,
						L".level\0*.level",
						L"../../_Textures",
						bind(&SceneEditor::OpenLevelFile, this, placeholders::_1),
						desc.Handle
					);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Save"))
			{
				if (ImGui::MenuItem(".trn"))
				{
					Path::SaveFileDialog
					(
						saveTerrainFile,
						L"Terrain_File\0*.trn",
						L"../../_Textures",
						bind(&Terrain::SaveTerrain, terrain, placeholders::_1),
						desc.Handle
					);
				}
				if (ImGui::MenuItem(".level"))
				{
					Path::SaveFileDialog
					(
						saveLevelFile,
						L".level\0*.level",
						L"../../_Textures",
						bind(&SceneEditor::SaveLevelFile, this, placeholders::_1),
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

void SceneEditor::Hiarachy()
{
	bool bOpen = true;
	bOpen = ImGui::Begin("Hiarachy", &bOpen);

	static ImGuiTextFilter filter;


	filter.Draw("Objects", 150.f);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
		{
			Transform* attachTransform = NULL;
			IM_ASSERT(payload->DataSize == sizeof(int));
			int payload_n = *(const int*)payload->Data;
			bool bCheck = false;
			for (int i = 0; i < hiarachyName.size(); i++)
			{
				if (hiarachyName[i] == modelNames[payload_n])
				{//Todo					
					attachTransform = modelLists[payload_n]->AddTransform();
					attachTransform->Position(50, 0, 50);
					attachTransform->Scale(0.1f, 0.1f, 0.1f);
					hiarachyName.push_back(modelNames[payload_n]);
					hiarachyMatrix.push_back(&attachTransform->World());
					bCheck = true;
					break;
				}
			}
			//처음받을떄 ?
			if (false == bCheck)
			{
				hiarachyName.push_back(modelNames[payload_n]);
				hiarachyMatrix.push_back(&modelLists[payload_n]->GetTransform(0)->World());
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::Separator();

	if (openModelFile != L"")
	{
		if (hiarachyName.size() != 0)
		{
			static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
			                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
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
				}
			}
		}
	}

	ImGui::End();
}

void SceneEditor::ViewModel()
{
	bool bOpen = true;
	D3DDesc desc = D3D::GetDesc();
	bOpen = ImGui::Begin("Project", &bOpen);
	//ImGui::SetWindowPos(ImVec2(width - windowWidth, 0));
	//ImGui::SetWindowSize(ImVec2(windowWidth, height));
	
	if (ImGui::Button("StaticMesh Load"))
	{
		Path::SaveFileDialog
		(
			openModelFile,
			L"Model_File\0*.mesh",
			L"../../_Models",
			bind(&SceneEditor::StaticMeshLoad, this, placeholders::_1),
			desc.Handle
		);
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (openModelFile != L"")
		{
			DragAndDropTreeNode("Meshes");
		}
	}
	ImGui::End();
}

void SceneEditor::OpenHeightMap(wstring file)
{
	if (terrain != NULL)
	{
		SafeDelete(terrain)
	}
	wstring folderName = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileName(file);

	terrain = new Terrain(terrainShader, folderName + L"/" + fileName);
	terrain->BaseMap(L"Terrain/Dirt3.png");
	terrain->LayerMap(L"Terrain/Cliff (Layered Rock).jpg", L"Terrain/Splatting.png");
}

void SceneEditor::OpenTrnFile(wstring file)
{
	if (terrain != NULL)
	{
		SafeDelete(terrain)
	}
	vector<float> terrainVertex;

	BinaryReader* r = new BinaryReader();
	r->Open(file);

	heightMapName = String::ToWString(r->String());
	wstring folderName = Path::GetLastDirectoryName(heightMapName);
	wstring fileName = Path::GetFileName(heightMapName);

	terrain = new Terrain(terrainShader, folderName + L"/" + fileName);
	terrain->BaseMap(String::ToWString(r->String()));
	wstring layerMapName = String::ToWString(r->String());//layerMap
	wstring alphaMapName = String::ToWString(r->String());//AlphaMap
	terrain->LayerMap(layerMapName, alphaMapName);

	UINT vertexCount = r->UInt();
	for (UINT i = 0; i < vertexCount; i++)
	{
		terrainVertex.push_back(r->Float());
	}
	terrain->ChangeVertex(terrainVertex);

	r->Close();
	SafeDelete(r);
}

void SceneEditor::OpenTerrainMapFile(wstring file)
{
	wstring s = Path::GetLastDirectoryName(file) + L"/";
	wstring s2 = Path::GetFileName(file);

	switch (terrainMapState)
	{
	case ETerrainMapState::eBaseMap:
		terrain->BaseMap(s + s2);
		break;
	case ETerrainMapState::eLayerMap:
		terrain->LayerMap(s + s2);
		break;
	case ETerrainMapState::eAlphaMap:
		terrain->AlphaMap(s + s2);
		break;
	default:
		break;
	}
}

void SceneEditor::StaticMeshLoad(wstring file)
{

	wstring fileDirectory = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileNameWithoutExtension(file);
	wstring fileFullDirectory = Path::GetDirectoryName(file);

	openModelFile = Path::GetFileNameWithoutExtension(file);
	
	if (!modelNames.empty())
	{
		for (int i = 0; i < modelNames.size(); ++i)
		{
			if (modelNames[i] == String::ToString(openModelFile))
			{
				assert(false, "Same ModelNames");
				return;
			}
		}
	}

	modelNames.push_back(String::ToString(openModelFile));
	ModelRender* modelRender = new ModelRender(modelShader);
	modelRender->ReadMaterial(fileDirectory + L"/" + fileName);
	modelRender->ReadMesh(fileDirectory + L"/" + fileName);

	Transform* attachTransform = modelRender->AddTransform();
	attachTransform->Position(50, 0, 50);
	attachTransform->Scale(0.1f, 0.1f, 0.1f);

	modelRender->UpdateTransforms();
	modelRender->Pass(1);

	modelLists.push_back(modelRender);
}

void SceneEditor::BillboardSet()
{
	std::string path("../../_Textures/Billboard");
	std::string ext(".png");
	billBoard = new Billboard(shader_56);

	for (auto& p : std::experimental::filesystem::recursive_directory_iterator(path))
	{
		if (p.path().extension() == ext)
		{
			billBoardNames.push_back(p.path().filename().c_str());
		}
	}
	for (wstring name : billBoardNames)
	{
		billBoard->AddTexture(L"Billboard/" + name);
		textureList.push_back(new Texture(L"Billboard/" + name));
	}
}

void SceneEditor::SaveLevelFile(wstring file)
{
//.level 에저장해야할것
//terrain 
//ModelRender 
//water  
//Billboard 
//Sky
//ModelMulti
//ModelAnim
	
	Path::CreateFolders(Path::GetDirectoryName(file));
	BinaryWriter* w = new BinaryWriter();
	w->Open(file);
	//terrain 정보
	if (nullptr != terrain)
	{
		w->String(terrain->heightMapFileName);

		wstring s = Path::GetLastDirectoryName((terrain->baseMap)->GetFile()) + L"/";
		wstring s2 = Path::GetFileName((terrain->baseMap)->GetFile());
		w->String(String::ToString(s + s2)); //baseMap

		s = Path::GetLastDirectoryName((terrain->layerMap)->GetFile()) + L"/";
		s2 = Path::GetFileName((terrain->layerMap)->GetFile());
		w->String(String::ToString(s + s2)); //layerMap

		s = Path::GetLastDirectoryName((terrain->alphaMap)->GetFile()) + L"/";
		s2 = Path::GetFileName((terrain->alphaMap)->GetFile());
		w->String(String::ToString(s + s2)); //alphaMap

		w->UInt(terrain->vertexCount);
		for (UINT i = 0; i < terrain->vertexCount; i++)
		{
			w->Float(terrain->vertices[i].Position.y);
		}
	}
	for (const auto modelRender : modelLists)
	{
		//행렬
		//머터리얼
		//매쉬
		for (UINT i = 0; i < modelRender->GetTransformCount(); ++i)
		{
			w->Matrix(modelRender->GetTransform(i)->World());
		}
		w->String(String::ToString(modelRender->GetModel()->GetMatFileName()));
		w->String(String::ToString(modelRender->GetModel()->GetMeshFileName()));
	}


	///////

	w->Close();
	SafeDelete(w);
}

void SceneEditor::OpenLevelFile(wstring file)
{
}

void SceneEditor::Inspector()
{
	ImGui::Begin("Inspector");
	if (ImGui::Button("Terrain"))
	{
		InspectorState = EInsPectorState::eTerrainEdit;
	}
	ImGui::SameLine();
	if (ImGui::Button("sky"))
	{
		InspectorState = EInsPectorState::eSkyEdit;
	}
	ImGui::SameLine();
	if (ImGui::Button("Billboard"))
	{
		InspectorState = EInsPectorState::eBillboardEdit;
	}

	if (NULL != terrain && (EInsPectorState::eTerrainEdit == InspectorState))
	{
		TerrainInspector();
	}

	if (NULL != sky && (EInsPectorState::eSkyEdit == InspectorState))
	{
		SkyInspector();
	}

	if (NULL != terrain && (EInsPectorState::eBillboardEdit == InspectorState))
	{
		BillboardInspector();
	}
	
	ImGui::End();
}

void SceneEditor::TerrainInspector()
{
	if (ImGui::CollapsingHeader("Brush", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();

		//Brush
		ImGui::InputInt("Type", (int*)& terrain->GetbrushDesc().Type);
		terrain->GetbrushDesc().Type %= 3;

		ImGui::InputInt("Range", (int*)& terrain->GetbrushDesc().Range);
		terrain->GetbrushDesc().Range %= 20;

		ImGui::Separator();
		ImGui::Checkbox("Noise", (bool*)& terrain->GetbrushDesc().Noise);

		ImGui::Checkbox("Smooth", (bool*)& terrain->GetbrushDesc().Smooth);
		ImGui::Checkbox("Flat", (bool*)& terrain->GetbrushDesc().Flat);
		ImGui::Checkbox("Slope", (bool*)& terrain->GetbrushDesc().Slope);
	}

	if (ImGui::CollapsingHeader("Line", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();
		//Line
		ImGui::ColorEdit3("Color", terrain->GetLineDesc().Color);

		ImGui::InputInt("Visible", (int*)& terrain->GetLineDesc().Visible);
		terrain->GetLineDesc().Visible %= 2;

		ImGui::InputFloat("Thickness", &terrain->GetLineDesc().Thickness, 0.001f);
		terrain->GetLineDesc().Thickness = Math::Clamp(terrain->GetLineDesc().Thickness, 0.01f, 0.9f);

		ImGui::InputFloat("Size", &terrain->GetLineDesc().Size, 1.0f);
		ImGui::Separator();
		ImGui::Button("DescHeight : leftShift + leftClick");
	}

	if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
	{
		D3DDesc desc = D3D::GetDesc();
		ImGui::Separator();
		ImGui::Separator();
		ImGui::Columns(3, "grid", true);
		ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		for (UINT colIndex = 0; colIndex < 3; colIndex++)
		{
			ImGui::SetColumnWidth(colIndex, 88);
		}

		//baseMap
		if (ImGui::ImageButton(*terrain->GetBaseMap(), ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), -1, color))
		{
			terrainMapState = ETerrainMapState::eBaseMap;
			Path::OpenFileDialog
			(
				openTerrainMapFile,
				Path::ImageFilter,
				L"../../_Textures",
				bind(&SceneEditor::OpenTerrainMapFile, this, placeholders::_1),
				desc.Handle
			);
		}
		ImGui::NextColumn();

		//layerMap
		if (ImGui::ImageButton(*terrain->GetLayerMap(), ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), -1, color))
		{
			terrainMapState = ETerrainMapState::eLayerMap;
			Path::OpenFileDialog
			(
				openTerrainMapFile,
				Path::ImageFilter,
				L"../../_Textures",
				bind(&SceneEditor::OpenTerrainMapFile, this, placeholders::_1),
				desc.Handle
			);
		}
		ImGui::NextColumn();
		//alphaMap
		if (ImGui::ImageButton(*terrain->GetAlphaMap(), ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), -1, color))
		{
			terrainMapState = ETerrainMapState::eAlphaMap;
			Path::OpenFileDialog
			(
				openTerrainMapFile,
				Path::ImageFilter,
				L"../../_Textures",
				bind(&SceneEditor::OpenTerrainMapFile, this, placeholders::_1),
				desc.Handle
			);
		}

	}
}

void SceneEditor::SkyInspector()
{
	//dome
	static Vector3 domePosition(0, 0, 0);
	static Vector3 domeScale(100, 100, 100);

	//moon
	static float theta = 0;
	static float moonDistance = 68.0f;
	static float moonGlowDistance = 67.0f;

	//cloud
	static float tile = 1.5f;
	static float cover = 0.005f;
	static float sharpness = 0.405f;
	static float speed = 0.05f;

	if (ImGui::CollapsingHeader("Sky", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();

		ImGui::SliderFloat("SunAngle", &theta, -Math::PI, Math::PI);
		sky->Theata(theta);
	}
	if (ImGui::CollapsingHeader("Dome", ImGuiTreeNodeFlags_DefaultOpen))
	{

		ImGui::Separator();
		ImGui::Separator();

		ImGui::SliderFloat3("DomePosition", domePosition, 0, 200.0f);
		ImGui::SliderFloat3("DomeScale", domeScale, 0, 200.0f);
		sky->GetDome()->GetTransform()->Position(domePosition);
		sky->GetDome()->GetTransform()->Scale(domeScale);
	}

	if (ImGui::CollapsingHeader("Moon", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();

		ImGui::SliderFloat("MoonDistance", &moonDistance, 60.0f, 200.0f);
		ImGui::SliderFloat("MoonGlow", &moonGlowDistance, 60.0f, 200.0f);
		sky->GetMoon()->GetDistance() = moonDistance;
		sky->GetMoon()->GetGlowDistance() = moonGlowDistance;
	}
	if (ImGui::CollapsingHeader("Cloud", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();

		ImGui::SliderFloat("Tile", &tile, 0.0f, 3.0f);
		ImGui::SliderFloat("Cover", &cover, 0.0f, 1.0f);
		ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, 1.0f);
		ImGui::SliderFloat("Speed", &speed, 0.0f, 1.0f);

		sky->GetCloud().Tile = tile;
		sky->GetCloud().Cover = cover;
		sky->GetCloud().Sharpness = sharpness;
		sky->GetCloud().Speed = speed;
	}
}

void SceneEditor::BillboardInspector()
{
	if (ImGui::CollapsingHeader("BillboardTree", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();

		ImGui::Columns(3, "grid", true);

		for (UINT colIndex = 0; colIndex < 3; colIndex++)
		{
			ImGui::SetColumnWidth(colIndex, 88);
		}


		for (UINT i = 0; i < textureList.size(); i++)
		{
			ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			if (i == billBoardCurID)
			{
				color.w = 0.3f;
			}
			if (ImGui::ImageButton(*textureList[i], ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), -1, color))
			{
				billBoardCurID = i;
			}
			ImGui::NextColumn();
		}
		if (Mouse::Get()->Down(0) && billBoardCurID != -1)
		{
			Vector2 scale = Math::RandomVec2(5, 10);
			Vector3 position = terrain->GetPickedPosition();

			billBoard->Add(Vector3(position.x, position.y + 5, position.z), scale, billBoardCurID);
		}
	}
}

void SceneEditor::GuizmoUpdate(int clickedNodeNum)
{
	ImGuizmo::BeginFrame();

	ImGui::Begin("Editor");
	ImGui::Text("Camera");

	ImGui::Separator();
	ImGuizmo::SetID(0);

	if (modelLists.size() != 0)
	{
		EditTransform(Context::Get()->View(), Context::Get()->Projection(), &hiarachyMatrix[clickedNodeNum][0][0], lastUsing == 0);
	}

	ImGui::End();
}

void SceneEditor::DragAndDropTreeNode(const char* label)
{
	if ("Meshes" == label)
	{
		for (int n = 0; n < modelNames.size(); n++)
		{
			ImGui::PushID(n);
			if (ImGui::TreeNode(modelNames[n].c_str()))
			{
				ImGui::TreePop();
			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				ImGui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));    // Set payload to carry the index of our item (could be anything)

				ImGui::Text("%s", modelNames[n].c_str());
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

}