#include "stdafx.h"
#include "QuadTreeFrustum.h"
#include "Viewer/Freedom.h"
#include "../Framework/Objects/QuadTree.h"
void QuadTreeFrustum::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(103, 136, -89);
	((Freedom*)Context::Get()->GetCamera())->Speed(60, 2);


	terrainShader = new Shader(L"21_Terrain_Brush.fxo");
	terrain = new Terrain(terrainShader, L"Terrain/Gray256.png");
	terrain->BaseMap(L"Terrain/Dirt3.png");
	Transform* transform = terrain->GetTransform();
	transform->Position(0,0,0);
	
    //texture, vs,ps sha


	camera = new Fixity();
	camera->Position(100, 0, 0);
	perspective = new Perspective(1024, 768, 1, zFar, Math::PI * fov);
	
	frustum = new Frustum(camera, perspective);

	quadTree = new QuadTree(terrainShader,frustum);
	quadTree->Init(terrain);

	shader = new Shader(L"47_CpuFrustum.fxo");
	perFrame = new PerFrame(shader);

	GuiSet();

}

void QuadTreeFrustum::Destroy()
{

	SafeDelete(shader);
	SafeDelete(perFrame);

	SafeDelete(terrainShader);
	SafeDelete(terrain);

	SafeDelete(camera);
	SafeDelete(perspective);
	SafeDelete(frustum);


}

void QuadTreeFrustum::Update()
{
	static float cameraX = camera->GetPosition().x;
	static float cameraY = camera->GetPosition().y;
	static float cameraZ = camera->GetPosition().z;
	
	static Vector3 rotation = camera->GetRotation();

	ImGui::Begin("Debug");
	{
		ImGui::TextColored(ImVec4(0,0,9,1), "Frustum");
		ImGui::SliderFloat("zFar", &zFar, 1.0f,500.0f);
		ImGui::SliderFloat("FOV", &fov, 1e-3f,1.0f);
		ImGui::SliderFloat("width", &width, 1.0f,2000.0f);
		ImGui::SliderFloat("height", &height, 1.0f, 1000.0f);
		ImGui::TextColored(ImVec4(0, 0, 9, 1), "Position");
		ImGui::PushItemWidth(100);
		ImGui::SliderFloat("x", &cameraX, -300.0f, 300.0f);
		ImGui::SameLine();				   
		ImGui::SliderFloat("y", &cameraY, -300.0f, 300.0f);
		ImGui::SameLine();				   
		ImGui::SliderFloat("z", &cameraZ, -300.0f, 300.0f);
		ImGui::NewLine();
		ImGui::TextColored(ImVec4(0, 0, 9, 1), "Rotation");
		ImGui::SliderFloat("xAxis", &rotation.x, -180.0f+ 1e-6f, 180.0f - 1e-6f);
		ImGui::SameLine();
		ImGui::SliderFloat("yAxis", &rotation.y, -180.0f + 1e-6f, 180.0f - 1e-6f);
		ImGui::SameLine();
		ImGui::SliderFloat("zAxis", &rotation.z, -180.0f + 1e-6f, 180.0f - 1e-6f);
	}
	ImGui::End();
	camera->Position(cameraX, cameraY, cameraZ);
	camera->RotationDegree(rotation);
	
	perspective->Set(width, height, 1, zFar, Math::PI * fov);
	
	frustum->Update();
	perFrame->Update();

	terrain->Update();
	quadTree->Update();
	camera->Update();
}

void QuadTreeFrustum::Render()
{
	//floor->Render();
	//grid->Render();
	frustum->Render();
	//wireFrame mode // ImGui 
	//terrain->Pass(1);
	//terrain->Render();
	quadTree->Render();
	
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	perFrame->Render();
	//vertexBuffer->Render();
	//indexBuffer->Render();



	UINT drawCount = 0;
	Vector3 position;
	for (Transform* transform : transforms)
	{
		transform->Position(&position);

		if (frustum->CheckPoint(position) == true)
		{
			transform->Update();
			transform->Render();

			shader->DrawIndexed(0, 0, 36);

			drawCount++;
		}
	}

	string str = "TriangleCount : " + to_string(quadTree->GetDrawCount());
	Gui::Get()->RenderText(10, 60, 0, 1, 1, str);

	Plane planes[6];
	frustum->Planes(planes);
}

void QuadTreeFrustum::GuiSet()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

}


