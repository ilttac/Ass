#include "Framework.h"
#include "Dome.h"

Dome::Dome(Shader* shader, Vector3 position, Vector3 scale, UINT drawCount)
	:Renderer(shader),drawCount(drawCount)
{
	GetTransform()->Position(position);
	GetTransform()->Scale(scale);

	UINT latitude = drawCount / 2; //위도
	UINT longtitude = drawCount; //경도

	vertexCount = longtitude * latitude * 2;
	indexCount = (longtitude - 1) * (latitude - 1) * 2 * 8;

	VertexTexture* vertices = new VertexTexture[vertexCount];

	UINT index = 0;
	for (UINT i = 0; i < longtitude; i++)
	{
		float xz = 100.0f * (i / (longtitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = Math::PI * j / (latitude - 1);
			vertices[index].Position.x = sinf(xz)* cosf(y);
			vertices[index].Position.y =cosf(xz);
			vertices[index].Position.z = sinf(xz)*sinf(y);

			vertices[index].Uv.x = 0.5f / (float)longtitude + i / (float)longtitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		}
	}
}

Dome::~Dome()
{
}

void Dome::Update()
{
}

void Dome::Render()
{
}
