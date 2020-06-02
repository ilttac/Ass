#pragma once

class GBuffer
{
public:
	GBuffer(Shader* shader, UINT width = 0, UINT height = 0);
	~GBuffer();

	void PackGBuffer();	
	void Lighting();	
	void DebugRender();
	
private:
	void CreateDepthStencilView();
	void CreateDepthStencilState();
	void CreateRasterierState();
	
private:
	struct Desc
	{
		Vector4 Perspective;
		Vector2 PowerRange = Vector2(1e-6f, 15.0f);		
		float Padding[2];
	} desc;

	struct PointLightDesc
	{
		float TessFator = 16.0f;
		float Padding[3];

		Matrix Projection;
		
		PointLight PointLight;
	} pointLightDesc;

private:
	Shader * shader;
	UINT width, height;
	
	RenderTarget* diffuseRTV; //RGB(24), A(8)
	RenderTarget* specularRTV; //RGB(24), A(8)
	RenderTarget* emissiveRTV; //RGB(24), Pow(8)
	RenderTarget* normalRTV; //Normal(32)
	RenderTarget* tangentRTV; //Tangent(32)
	DepthStencil* depthStencil;
	Viewport* viewport;

	ID3D11DepthStencilView* depthStencilReadOnly;

	ID3D11DepthStencilState* packDss;
	ID3D11DepthStencilState* noDepthWriteLessDSS;
	ID3D11DepthStencilState* noDepthWriteGreaterDSS;
	ID3DX11EffectDepthStencilVariable* sDSS;

	ID3D11RasterizerState* debugRSS;
	ID3D11RasterizerState* lightRSS;
	ID3DX11EffectRasterizerVariable* sRSS;
	
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
	ID3DX11EffectShaderResourceVariable* sSrvs;

	PointLight pointLights[MAX_POINT_LIGHT];
	ConstantBuffer* pointLightBuffer;
	ID3DX11EffectConstantBuffer* sPointLightBuffer;

	Render2D* debug2D[6];
};
