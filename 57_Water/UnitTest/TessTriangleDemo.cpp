#include "stdafx.h"
#include "TessTriangleDemo.h"

void TessTriangleDemo::Initialize()
{
	shader = new Shader(L"46_TessTriangle.fx");
	
	Context::Get()->GetCamera()->RotationDegree(0, 0, 0);
	Context::Get()->GetCamera()->Position(0, 0, -5);
	((Freedom*)Context::Get()->GetCamera())->Speed(10, 0);


	Vertex vertices[3];
	vertices[0].Position = Vector3(-0.9f, -0.9f, 0.0f);
	vertices[1].Position = Vector3(+0.0f, +0.9f, 0.0f);
	vertices[2].Position = Vector3(+0.9f, -0.9f, 0.0f);
	
	vertexBuffer = new VertexBuffer(vertices, 3, sizeof(Vertex));
}

void TessTriangleDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(vertexBuffer);
}

void TessTriangleDemo::Update()
{
	static Vector4 vec = Vector4(2, 2, 2, 1);
	ImGui::SliderFloat4("Factor", vec, 1, 64);
	shader->AsVector("Factor")->SetFloatVector(vec);
}

void TessTriangleDemo::Render()
{
	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	
	shader->Draw(0, 0, 3);
}


