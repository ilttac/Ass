#include "Framework.h"
#include "Dome.h"

Dome::Dome(Shader* shader, Vector3 position, Vector3 scale, UINT drawCount)
	:Renderer(shader),drawCount(drawCount)
{
	starMap = new Texture(L"Environment/Starfield.png");
	sStarMap = shader->AsSRV("StarMap");

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
		}//for(i)
	}//for(j)

	for (UINT i = 0; i < longtitude; i++)
	{
		float xz = 100.0f * (i / (longtitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = (Math::PI * 2.0f) - (Math::PI *j / (latitude -1));

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);

			vertices[index].Uv.x = 0.5f / (float)longtitude + i / (float)longtitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		}
	}

	index = 0;
	UINT* indices = new UINT[indexCount * 3];

	for (UINT i = 0; i < longtitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = i * latitude + j;
			indices[index++] = (i + 1) * latitude + j;
			indices[index++] = (i + 1) * latitude + j+1 ;

			indices[index++] =  i *latitude + j+1 ;
			indices[index++] = i*latitude + j ;
			indices[index++] = (i+1) * latitude+j+1;
		}
	}

	UINT offset = latitude * longtitude;
	for (UINT i = 0; i < longtitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = offset + i * latitude + j;
			indices[index++] = offset + (i+1) * latitude + (j+1);
			indices[index++] = offset + (i+1) * latitude + j;

			indices[index++] = offset + i * latitude + (j+1);
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + i * latitude + j;
		}
	}
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));
	indexBuffer = new IndexBuffer(indices, indexCount);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}

Dome::~Dome()
{
}

void Dome::Update()
{
	Super::Update();
}

void Dome::Render()
{
	Super::Render();

	sStarMap->SetResource(starMap->SRV());
	shader->DrawIndexed(0, Pass(), indexCount);
}
