#include "stdafx.h"
#include "SceneEditor.h"
#include "Utilities/Xml.h"
#include "../Framework/Utilities/BinaryFile.h"

void SceneEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	shader = new Shader(L"23_TerrainSpatting.fxo");
	//skyShader = new Shader(L"57_ParticleViewer.fxo");

	//shadow = new Shadow(skyShader, Vector3(0, 0, 0), 65);
	
	//Terrain
	//terrain = new Terrain(shader, L"Terrain/TestMap.png");
	//terrain->BaseMap(L"Terrain/Dirt3.png");
	//terrain->LayerMap(L"Terrain/Cliff (Layered Rock).jpg", L"Terrain/Splatting.png");
	//
	/*sky = new Sky(skyShader);
	sky->ScatteringPass(3);
	sky->RealTime(false, Math::PI - 1e-6f, 0.5f);*/
}

void SceneEditor::Destroy()
{
	SafeDelete(shader);
	SafeDelete(shadow);

	SafeDelete(sky);

	SafeDelete(terrain);
}

void SceneEditor::Update()
{

	//sky->Update();
	if (terrain != NULL)
	{
		terrain->Update();
	}
	MainMenu();
	Inspector();
}

void SceneEditor::PreRender()
{
	//sky->PreRender();

	////Depth
	//{
	//	shadow->Set();
	//	//sphere->Render();
	//}
}

void SceneEditor::Render()
{
//	sky->Pass(4, 5, 6);
	//sky->Render();
	if (terrain != NULL)
	{
		terrain->Render();
	}
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
				if(ImGui::MenuItem(".png"))
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
						bind(&Terrain::SaveTerrain, terrain,  placeholders::_1),
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

void SceneEditor::OpenHeightMap(wstring file)
{
	if (terrain != NULL)
	{
		SafeDelete(terrain)
	}
	wstring folderName = Path::GetLastDirectoryName(file);
	wstring fileName = Path::GetFileName(file);

	terrain = new Terrain(shader, folderName + L"/"+ fileName);
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

	terrain = new Terrain(shader, folderName + L"/" + fileName);
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

void SceneEditor::Inspector()
{
	ImGui::Begin("Inspector");
	if (NULL != terrain)
	{
		TerrainInspector();
	}
	ImGui::End();
}

void SceneEditor::TerrainInspector()
{
	if (ImGui::CollapsingHeader("Brush"), ImGuiTreeNodeFlags_DefaultOpen)
	{
		//Brush
		ImGui::InputInt("Type", (int*)&terrain->GetbrushDesc().Type);
		terrain->GetbrushDesc().Type %= 3;

		ImGui::InputInt("Range", (int*)& terrain->GetbrushDesc().Range);
		terrain->GetbrushDesc().Range %= 20;

		ImGui::Separator();
		ImGui::Checkbox("Noise", (bool*)& terrain->GetbrushDesc().Noise);

		ImGui::Checkbox("Smooth", (bool*)& terrain->GetbrushDesc().Smooth);
		ImGui::Checkbox("Flat", (bool*)& terrain->GetbrushDesc().Flat);
		ImGui::Checkbox("Slope", (bool*)& terrain->GetbrushDesc().Slope);
	}
	if (ImGui::CollapsingHeader("Line"), ImGuiTreeNodeFlags_DefaultOpen)
	{
		//Line
		ImGui::Separator();
		ImGui::ColorEdit3("Color", terrain->GetLineDesc().Color);

		ImGui::InputInt("Visible", (int*)& terrain->GetLineDesc().Visible);
		terrain->GetLineDesc().Visible %= 2;

		ImGui::InputFloat("Thickness", &terrain->GetLineDesc().Thickness, 0.001f);
		terrain->GetLineDesc().Thickness = Math::Clamp(terrain->GetLineDesc().Thickness, 0.01f, 0.9f);

		ImGui::InputFloat("Size", &terrain->GetLineDesc().Size, 1.0f);
		ImGui::Separator();
		ImGui::Button("DescHeight : leftShift + leftClick"); 
	}
}
