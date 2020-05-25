#include "Framework.h"
#include "GBuffer.h"

GBuffer::GBuffer(Shader* shader, UINT width, UINT height)
	: shader(shader)
{
	this->width = width < 1 ? (UINT)D3D::Width() : width;
	this->height = height < 1 ? (UINT)D3D::Height() : height;


	diffuseRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	specularRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	emissiveRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	normalRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	tangentRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R32G32B32A32_FLOAT);

	depthStencil = new DepthStencil(this->width, this->height, true);

	viewport = new Viewport((float)this->width, (float)this->height);

	sSrvs = shader->AsSRV("GBufferMaps");

	pointLightBuffer = new ConstantBuffer(&pointLightDesc, sizeof(PointLightDesc));
	sPointLightBuffer = shader->AsConstantBuffer("CB_Deffered_PointLight");

	spotLightBuffer = new ConstantBuffer(&spotLightDesc, sizeof(SpotLightDesc));
	sSpotLightBuffer = shader->AsConstantBuffer("CB_Deffered_SpotLight");

	sDSS = shader->AsDepthStencil("Deffered_DepthStencil_State");
	sRSS = shader->AsRasterizer("Deffered_Rasterizer_State");

	CreateDepthStencilView();
	CreateDepthStencilState();
	CreateRasterizerState();

	for (UINT i = 0; i < 6; i++)
	{
		debug2D[i] = new Render2D();
		debug2D[i]->GetTransform()->Position(75 + (float)i * 150, 75, 0);
		debug2D[i]->GetTransform()->Scale(150, 150, 0);
	}
	debug2D[0]->SRV(diffuseRTV->SRV());
	debug2D[1]->SRV(specularRTV->SRV());
	debug2D[2]->SRV(emissiveRTV->SRV());
	debug2D[3]->SRV(normalRTV->SRV());
	debug2D[4]->SRV(tangentRTV->SRV());
	debug2D[5]->SRV(depthStencil->SRV());

}

GBuffer::~GBuffer()
{
	SafeDelete(diffuseRTV);
	SafeDelete(specularRTV);
	SafeDelete(emissiveRTV);
	SafeDelete(normalRTV);
	SafeDelete(tangentRTV);
	SafeDelete(depthStencil);
	SafeDelete(viewport);

	SafeDelete(pointLightBuffer);
	SafeDelete(spotLightBuffer);

	SafeRelease(depthStencilReadOnly);
	SafeRelease(packDss);
	SafeRelease(noDepthWriteLessDSS);
	SafeRelease(noDepthWriteGreaterDSS);

	SafeRelease(debugRSS);
	SafeRelease(lightRSS);


	for (UINT i = 0; i < 6; i++)
		SafeDelete(debug2D[i]);
}

void GBuffer::PackGBuffer()
{
	RenderTarget* rtvs[5];
	rtvs[0] = diffuseRTV;
	rtvs[1] = specularRTV;
	rtvs[2] = emissiveRTV;
	rtvs[3] = normalRTV;
	rtvs[4] = tangentRTV;

	RenderTarget::Sets(rtvs, 5, depthStencil);
	viewport->RSSetViewport();

	sDSS->SetDepthStencilState(0, packDss);
}

void GBuffer::Lighting()
{
	D3D::Get()->SetRenderTarget(NULL, depthStencilReadOnly);
	D3D::GetDC()->ClearDepthStencilView(depthStencilReadOnly, D3D11_CLEAR_DEPTH, 1, 0);

	ID3D11ShaderResourceView* srvs[6] =
	{
		depthStencil->SRV(),
		diffuseRTV->SRV(),
		specularRTV->SRV(),
		emissiveRTV->SRV(),
		normalRTV->SRV(),
		tangentRTV->SRV(),
	};
	sSrvs->SetResourceArray(srvs, 0, 6);
	D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);

	//Directional
	{
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		sDSS->SetDepthStencilState(0, noDepthWriteLessDSS);
		shader->Draw(0, 3, 4);
	}

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	sPointLightBuffer->SetConstantBuffer(pointLightBuffer->Buffer());

	//PointLight - Debug
	{
		if (bDebug == true)
		{
			ImGui::InputFloat("PointLight Factor", &pointLightDesc.TessFator, 1.0f);

			sRSS->SetRasterizerState(0, debugRSS);

			UINT count = Context::Get()->PointLights(pointLightDesc.PointLight);
			for (UINT i = 0; i < count; i++)
			{
				Matrix S, T;
				float s = pointLightDesc.PointLight[i].Range;
				Vector3 t = pointLightDesc.PointLight[i].Position;

				D3DXMatrixScaling(&S, s, s, s);
				D3DXMatrixTranslation(&T, t.x, t.y, t.z);

				pointLightDesc.Projection[i] = S * T * Context::Get()->View() * Context::Get()->Projection();
			}
			pointLightBuffer->Apply();

			shader->Draw(0, 4, 2 * count);
		}
	
	}

	//PointLight
	{
		sRSS->SetRasterizerState(0, lightRSS);
		sDSS->SetDepthStencilState(0, noDepthWriteGreaterDSS);

		UINT count = Context::Get()->PointLights(pointLightDesc.PointLight);
		for (UINT i = 0; i < count; i++)
		{
			Matrix S, T;
			float s = pointLightDesc.PointLight[i].Range;
			Vector3 t = pointLightDesc.PointLight[i].Position;

			D3DXMatrixScaling(&S, s, s, s);
			D3DXMatrixTranslation(&T, t.x, t.y, t.z);

			pointLightDesc.Projection[i] = S * T * Context::Get()->View() * Context::Get()->Projection();

		}
		pointLightBuffer->Apply();
		shader->Draw(0, 5, count*2);
	}
}

void GBuffer::DebugRender()
{
	for (int i = 0; i < 6; i++)
	{
		debug2D[i]->Update();
		debug2D[i]->Render();
	}
}

void GBuffer::CreateDepthStencilView()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	desc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;

	Check(D3D::GetDevice()->CreateDepthStencilView(depthStencil->BackBuffer(), &desc, &depthStencilReadOnly));
}

void GBuffer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = TRUE;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	//Pack
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp =
	{
		D3D11_STENCIL_OP_REPLACE,
		D3D11_STENCIL_OP_REPLACE,
		D3D11_STENCIL_OP_REPLACE,
		D3D11_COMPARISON_ALWAYS
	};
	desc.FrontFace = stencilMarkOp;
	desc.BackFace = stencilMarkOp;

	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &packDss));

	//No Depth Write Less
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp2 =
	{
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
		D3D11_COMPARISON_EQUAL
	};
	desc.FrontFace = stencilMarkOp2;
	desc.BackFace = stencilMarkOp2;

	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &noDepthWriteLessDSS));

	//No Depth Write Greater
	desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;

	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &noDepthWriteGreaterDSS))
}

void GBuffer::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));

	desc.FillMode = D3D11_FILL_WIREFRAME;
	desc.CullMode = D3D11_CULL_BACK;
	Check(D3D::GetDevice()->CreateRasterizerState(&desc, &debugRSS));

	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_FRONT;
	Check(D3D::GetDevice()->CreateRasterizerState(&desc, &lightRSS));
}

void GBuffer::CalcPointLight(UINT count)
{
}

void GBuffer::RenderPointLights()
{
}

void GBuffer::CalcSpotLight(UINT count)
{
}

void GBuffer::RenderSpotLights()
{
}
