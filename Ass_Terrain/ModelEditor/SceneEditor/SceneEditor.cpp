#include "stdafx.h"
#include "SceneEditor.h"
#include "Utilities/Xml.h"
#include "../Framework/Utilities/BinaryFile.h"
#include "../Framework/Environment/Sky/Dome.h"
#include "../Framework/Environment/Sky/Moon.h"
#include "../Framework/Environment/Sky/Cloud.h"

void SceneEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);

	terrainShader = new Shader(L"23_TerrainSpatting.fxo");
	shader_57 = new Shader(L"57_ParticleViewer.fxo");
	shader_53 = new Shader(L"53_DefferedShadow.fxo");
	gBuffer = new GBuffer(shader_53);
	//shadow = new Shadow(skyShader, Vector3(0, 0, 0), 65);

	//Terrain
	//terrain = new Terrain(terrainShader, L"Terrain/TestMap.png");
	//terrain->BaseMap(L"Terrain/Dirt3.png");
	//terrain->LayerMap(L"Terrain/Cliff (Layered Rock).jpg", L"Terrain/Splatting.png");
	//
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

	sky->Update();
	if (terrain != NULL)
	{
		terrain->Update();
	}
	MainMenu();
	Inspector();
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
	gBuffer->Render();

	sky->Pass(4, 5, 6);
	sky->Render();
	if (terrain != NULL)
	{
		terrain->Render();
	}
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

void SceneEditor::Inspector()
{
	ImGui::Begin("Inspector");
	if (NULL != terrain)
	{
		TerrainInspector();

	}
	if (NULL != sky)
	{
		SkyInspector();
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
