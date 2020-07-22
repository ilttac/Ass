#include "Framework.h"
#include "Cloud.h"

Cloud::Cloud(Shader * shader)
	: Renderer(shader)
{
	sSRV = shader->AsSRV("CloudMap");

	render2D = new Render2D();
	render2D->GetTransform()->Position(400 + 100, D3D::Height() - 100, 0);
	render2D->GetTransform()->Scale(200, 200, 1);

	VertexTexture vertices[6];
	vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[4].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[5].Position = Vector3(+1.0f, +1.0f, 0.0f);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 1);
	vertices[4].Uv = Vector2(0, 0);
	vertices[5].Uv = Vector2(1, 0);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(VertexTexture));

	CreateTexture();
}

Cloud::~Cloud()
{
	SafeRelease(texture);
	SafeRelease(srv);

	SafeDelete(render2D);
}

void Cloud::Update()
{
	Super::Update();

	Vector3 position(0, 0, 0);
	Context::Get()->GetCamera()->Position(&position);

	transform->Scale(1000, 1000, 10);
	transform->Rotation(-Math::PI * 0.5f, 0, 0);
	transform->Position(position.x + 95, position.y + 100.0f, position.z + 300.0f);
}

void Cloud::Render()
{
	Super::Render();

	/*UINT stride = sizeof(VertexTexture);
	UINT offset = 0;*/

	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	sSRV->SetResource(srv);
	shader->Draw(0, Pass(), 6);

}

void Cloud::PostRender()
{
	render2D->Update();
	render2D->SRV(srv);
	render2D->Render();
}

void Cloud::CreateTexture()
{
	int perm[] =
	{
		79,160,137,91,90,232,
		34,13,201,95,96,53,194,233,7,23,140,20,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,22,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,68, 232,175,74,165,9,134,139,48,27,166,
		77,146,158,231,83,8,229,122,60,11,123,232,220,105,232,41,232,46,245,40,244,
		102,143,232, 65,25,63,161, 1,216,83,73,56,76,132,232,232, 232,18,169,200,196,
		135,130,116,188,20,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,224,126,255,232,232,212,207,206,59,227,47,16,58,17,182,189,28,42,
		 19,20,13,9,224,113,224,232,178,15, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,255,255,255,255, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,232,106,157,255,255,255,255,115,121,50,45,127, 4,150,254,
		138,236,67,93,222,114,67,223,183,170,78,119,232,123, 232,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253,29,24,72,243,96,128,195,78,66,215,61,4,7,	79,160,137,91,90,232,
		34,13,201,95,96,53,194,233,7,23,140,20,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,22,219,203,117,35,11,32,57,177,33,
		88,255,255,56,87,174,20,125,136,171,68, 232,175,74,165,9,134,139,48,27,166,
		77,255,255,231,83,8,229,122  ,255,255,255,255, 220,105,232,41,232,46,245,40,244,
		102,255,232, 65,25,63,161, 1 ,255,255,255,255 , 76,132,232,232, 232,18,169,200,196,
		135,255,255,255,20,86,164,100,255,255,255  ,  186, 3,64,52,217,226,250,124,123,
		5,202,38,147,224,126,255,232,232,212,207,206,59,227,47,16,58,17,182,189,28,42,
		 19,20,13,9,224,113,224,232,178,15, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,232,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,67,93,222,114,67,223,183,170,78,119,232,123, 232,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253,29,24,72,243,96,128,195,78,66,215,61,4,7
	};

	int gradValues[] =
	{
		+1, +1, +1, -1, +1, +1, +1, -1, +1, -1, -1, +1,
		+1, +1, +1, -1, +1, +1, +1, +0, -1, +1, +0, -1,
		+0, +1, +1, +1, -1, +1, +0, +1, -1, +1, -1, +1,
		+1, +1, +1, +1, -1, +1, -1, +1, +1, +1, +1, +1
	};

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = 512;
	desc.Height = 512;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;


	Color* pixels = new Color[512 * 512];
	for (int y = 0; y < 512; y++)
	{
		for (int x = 0; x < 512; x++)
		{
			int value = perm[(x + perm[y]) & 0xFF];

			Color color;
			color.r = (float)gradValues[value & 0x0F] * 64 + 64;
			color.g = (float)gradValues[value & 0x0F + 1] * 64 + 64;
			color.b = (float)gradValues[value & 0x0F + 2] * 64 + 64;
			color.a = (float)value;

			UINT index = desc.Width * y + x;
			pixels[index] = color;
		}
	}

	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = 512 * 4;

	Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &texture));

	//D3DX11SaveTextureToFile(D3D::GetDC(), texture, D3DX11_IFF_PNG, L"Noise.png");
	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = desc.Format;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
	}
	
	SafeDeleteArray(pixels);
}
