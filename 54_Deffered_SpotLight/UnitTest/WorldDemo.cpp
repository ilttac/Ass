#include "stdafx.h"
#include "WorldDemo.h"

void WorldDemo::Initialize()
{
	shader = new Shader(L"04_World.fx");

	vertices[0].Position = Vector3(-0.5f, -0.5f, 0.0f);
	vertices[1].Position = Vector3(-0.5f, +0.5f, 0.0f);
	vertices[2].Position = Vector3(+0.5f, -0.5f, 0.0f);
	vertices[3].Position = Vector3(+0.5f, -0.5f, 0.0f);
	vertices[4].Position = Vector3(-0.5f, +0.5f, 0.0f);
	vertices[5].Position = Vector3(+0.5f, +0.5f, 0.0f);


	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = sizeof(Vertex) * 6;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = vertices;

	Check(D3D::GetDevice()->CreateBuffer(&desc, &subResource, &vertexBuffer));

	D3DXMatrixIdentity(&world);
	D3DXMatrixIdentity(&world2);
}

void WorldDemo::Destroy()
{
	SafeDelete(shader);
	SafeRelease(vertexBuffer);
}

void WorldDemo::Update()
{
	shader->AsMatrix("View")->SetMatrix(Context::Get()->View());
	shader->AsMatrix("Projection")->SetMatrix(Context::Get()->Projection());
}

void WorldDemo::Render()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//World
	{
		static Vector3 position = Vector3(0, 0, 0);
		ImGui::SliderFloat2("Position", position, -10, 10);

		world._41 = position.x;
		world._42 = position.y;

		Color color = Color(1, 0, 0, 1);

		shader->AsMatrix("World")->SetMatrix(world);
		shader->AsVector("Color")->SetFloatVector(color);

		shader->Draw(0, 0, 6);
	}

	
	//World2
	{
		static Vector3 position = Vector3(0, 0, 0);
		ImGui::SliderFloat3("Position2", position, -10, 10);

		world2._41 = position.x;
		world2._42 = position.y;
		world2._43 = position.z;

		Color color = Color(0, 0, 1, 1);

		shader->AsMatrix("World")->SetMatrix(world2);
		shader->AsVector("Color")->SetFloatVector(color);

		shader->Draw(0, 0, 6);
	}

}


