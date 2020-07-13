#include "stdafx.h"
#include "TessQuadDemo.h"

void TessQuadDemo::Initialize()
{
	shader = new Shader(L"46_TessQuad.fx");
	
	Context::Get()->GetCamera()->RotationDegree(0, 0, 0);
	Context::Get()->GetCamera()->Position(0, 0, -5);
	((Freedom*)Context::Get()->GetCamera())->Speed(10, 0);


	Vertex vertices[4];
	vertices[0].Position = Vector3(-0.9f, -0.9f, 0.0f);
	vertices[1].Position = Vector3(-0.9f, +0.9f, 0.0f);
	vertices[2].Position = Vector3(+0.9f, -0.9f, 0.0f);
	vertices[3].Position = Vector3(+0.9f, +0.9f, 0.0f);
	
	vertexBuffer = new VertexBuffer(vertices, 4, sizeof(Vertex));
}

void TessQuadDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(vertexBuffer);
}

void TessQuadDemo::Update()
{
	static Vector4 vec = Vector4(2, 2, 2, 2);
	ImGui::SliderFloat4("Factor", vec, 1, 64);
	shader->AsVector("Factor")->SetFloatVector(vec);

	static Vector2 inside = Vector2(1, 1);
	ImGui::SliderFloat2("Inside", inside, 1, 64);
	shader->AsVector("Inside")->SetFloatVector(inside);
}

void TessQuadDemo::Render()
{
	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	
	shader->Draw(0, 0, 4);
}


