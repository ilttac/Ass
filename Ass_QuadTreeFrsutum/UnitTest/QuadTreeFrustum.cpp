#include "stdafx.h"
#include "QuadTreeFrustum.h"
#include "Viewer/Freedom.h"
#include "../Framework/Objects/QuadTree.h"
void QuadTreeFrustum::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(0, 20, -70);
	((Freedom*)Context::Get()->GetCamera())->Speed(20, 2);

	gridShader = new Shader(L"31_Mesh.fxo");
	

	terrainShader = new Shader(L"21_Terrain_Brush.fxo");
	terrain = new Terrain(terrainShader, L"Terrain/Gray256.png");
	terrain->BaseMap(L"Terrain/Dirt3.png");
	Transform* transform = terrain->GetTransform();
	transform->Position(0,0,0);
	
    //texture, vs,ps sha


	floor = new Material(gridShader);
	floor->DiffuseMap(L"White.png");

	grid = new MeshRender(gridShader, new MeshGrid());
	grid->AddTransform()->Scale(10, 10, 10);
	grid->UpdateTransforms();

	camera = new Fixity();
	camera->Position(100, 0, 0);
	perspective = new Perspective(1024, 768, 1, zFar, Math::PI * fov);
	
	t = new Transform();
	t->Position(0, 0, -50);
	frustumCamera = new FrustumCamera(t);
	
	frustum = new Frustum(camera, perspective);

	quadTree = new QuadTree(terrainShader,frustum);
	quadTree->Init(terrain);

	shader = new Shader(L"47_CpuFrustum.fxo");
	perFrame = new PerFrame(shader);

	red = new Material(shader);
	red->DiffuseMap("Red.png");

	for (float z = -50.0f; z < 50.0f; z += 10)
	{
		for (float y = -50.0f; y < 50.0f; y += 10)
		{
			for (float x = -50.0f; x < 50.0f; x += 10)
			{
				Transform* transform = new Transform(shader);
				transform->Position(x, y, z);

				transforms.push_back(transform);
			}
		}
	}

	CreateMeshData();
	GuiSet();

	modelShader = new Shader(L"47_GpuFrustum.fxo");
	model = new ModelRender(modelShader);
	model->ReadMaterial(L"B787/Airplane");
	model->ReadMesh(L"B787/Airplane");
	
	for (float z = -100; z < 100; z += 30)
	{
		for (float y = -100; y < 100; y += 30)
		{
			for (float x = -100; x < 100; x += 30)
			{
				Transform* transform = model->AddTransform();
				transform->Position(x, y, z);
				transform->Scale(0.004f, 0.004f, 0.004f);
				transform->Rotation(0, Math::PI * 0.25f, 0);
			}
		}
	}
	model->UpdateTransforms();

}

void QuadTreeFrustum::Destroy()
{
	SafeDelete(gridShader);
	SafeDelete(shader);
	SafeDelete(perFrame);

	SafeDelete(terrainShader);
	SafeDelete(terrain);

	SafeDelete(red);
	SafeDelete(floor);

	SafeDelete(camera);
	SafeDelete(perspective);
	SafeDelete(frustum);

	SafeDelete(modelShader);
	SafeDelete(model);
}

void QuadTreeFrustum::Update()
{
	static float cameraX = camera->GetPosition().x;
	static float cameraY = camera->GetPosition().y;
	static float cameraZ = camera->GetPosition().z;

	ImGui::Begin("Debug");
	{
		ImGui::TextColored(ImVec4(0,0,1,1), "Frustum");
		ImGui::SliderFloat("zFar", &zFar, 1.0f,500.0f);
		ImGui::SliderFloat("FOV", &fov, 1e-3f,1.0f);
		ImGui::SliderFloat("width", &width, 1.0f,2000.0f);
		ImGui::SliderFloat("height", &height, 1.0f, 1000.0f);
		ImGui::TextColored(ImVec4(0, 0, 1, 1), "Position");
		ImGui::PushItemWidth(50);
		ImGui::SliderFloat("x", &cameraX, -100.0f, 100.0f);
		ImGui::SameLine();				   
		ImGui::SliderFloat("y", &cameraY, -100.0f, 100.0f);
		ImGui::SameLine();				   
		ImGui::SliderFloat("z", &cameraZ, -100.0f, 100.0f);
	}
	ImGui::End();
	camera->Position(cameraX, cameraY, cameraZ);
	//float diag = sqrt((height * height) + (width * width));
	//fov = 2 * atan((width) / (2 * zFar))*(180/Math::PI);
	perspective->Set(width, height, 1, zFar, Math::PI * fov);
	
	frustumCamera->Update();
	frustum->Update();
	perFrame->Update();
	grid->Update();

	terrain->Update();
	quadTree->Update();
	camera->Update();
	model->Update();
}

void QuadTreeFrustum::Render()
{
	floor->Render();
	grid->Render();
	frustum->Render();
	//wireFrame mode // ImGui 
	//terrain->Pass(1);
	//terrain->Render();
	quadTree->Render();
	
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	perFrame->Render();
	vertexBuffer->Render();
	indexBuffer->Render();

	red->Render();

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

	string str = "Draw : " + to_string(drawCount) + ", Toal : " + to_string(transforms.size());
	Gui::Get()->RenderText(10, 60, 0, 1, 1, str);

	Plane planes[6];
	frustum->Planes(planes);
	modelShader->AsVector("Planes")->SetFloatVectorArray((float*)planes, 0, 6);

	model->Pass(1);
	model->Render();
}

void QuadTreeFrustum::CreateMeshData()
{
	vector<Mesh::MeshVertex> v;

	float w, h, d;
	w = h = d = 0.5f;

	//Front
	v.push_back(Mesh::MeshVertex(-w, -h, -d, 0, 1, 0, 0, -1, 1, 0, 0));
	v.push_back(Mesh::MeshVertex(-w, +h, -d, 0, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, +h, -d, 1, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, -h, -d, 1, 1, 0, 0, -1, 1, 0, 0));

	//Back
	v.push_back(Mesh::MeshVertex(-w, -h, +d, 1, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, -h, +d, 0, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, +h, +d, 0, 0, 0, 0, 1, -1, 0, 0));
	v.push_back(Mesh::MeshVertex(-w, +h, +d, 1, 0, 0, 0, 1, -1, 0, 0));

	//Top
	v.push_back(Mesh::MeshVertex(-w, +h, -d, 0, 1, 0, 1, 0, 1, 0, 0));
	v.push_back(Mesh::MeshVertex(-w, +h, +d, 0, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, +h, +d, 1, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, +h, -d, 1, 1, 0, 1, 0, 1, 0, 0));

	//Bottom
	v.push_back(Mesh::MeshVertex(-w, -h, -d, 1, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, -h, -d, 0, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(Mesh::MeshVertex(+w, -h, +d, 0, 0, 0, -1, 0, -1, 0, 0));
	v.push_back(Mesh::MeshVertex(-w, -h, +d, 1, 0, 0, -1, 0, -1, 0, 0));

	//Left
	v.push_back(Mesh::MeshVertex(-w, -h, +d, 0, 1, -1, 0, 0, 0, 0, -1));
	v.push_back(Mesh::MeshVertex(-w, +h, +d, 0, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(Mesh::MeshVertex(-w, +h, -d, 1, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(Mesh::MeshVertex(-w, -h, -d, 1, 1, -1, 0, 0, 0, 0, -1));

	//Right
	v.push_back(Mesh::MeshVertex(+w, -h, -d, 0, 1, 1, 0, 0, 0, 0, 1));
	v.push_back(Mesh::MeshVertex(+w, +h, -d, 0, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(Mesh::MeshVertex(+w, +h, +d, 1, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(Mesh::MeshVertex(+w, -h, +d, 1, 1, 1, 0, 0, 0, 0, 1));

	Mesh::MeshVertex* vertices = new Mesh::MeshVertex[v.size()];
	UINT vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<Mesh::MeshVertex*>(vertices, vertexCount)
	);

	UINT indexCount = 36;
	UINT* indices = new UINT[indexCount]
	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Mesh::MeshVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
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


