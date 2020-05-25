#pragma once

class GBuffer
{
public:
	GBuffer(Shader* shader, UINT width = 0, UINT height = 0);
	~GBuffer();

	void PackGBuffer();	
	void Lighting();	
	void DebugRender();
	void SetDebug(bool val) { bDebug = val; }
	

private:
	void CreateDepthStencilView();
	void CreateDepthStencilState();
	void CreateRasterizerState();

private:
	void CalcPointLight(UINT count);
	void RenderPointLights();
	
	void CalcSpotLight(UINT count);
	void RenderSpotLights();

private:
	struct PointLightDesc
	{
		float TessFator = 16.0f;
		float Padding[3];

		Matrix Projection[MAX_POINT_LIGHT];
		PointLight PointLight[MAX_POINT_LIGHT];
	}pointLightDesc;

	struct SpotLightDesc
	{
		float TessFator = 16.0f;
		float Padding[3];

		Vector4 Angle[MAX_SPOT_LIGHT];
		Matrix Projection[MAX_SPOT_LIGHT];
		SpotLight SpotLight[MAX_SPOT_LIGHT];
	}spotLightDesc;

private:
	bool bDebug = false;

private:
	Shader * shader;
	UINT width, height;
	
	RenderTarget* diffuseRTV;  
	RenderTarget* specularRTV; 
	RenderTarget* emissiveRTV; 
	RenderTarget* normalRTV;   
	RenderTarget* tangentRTV;  
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

	ID3DX11EffectShaderResourceVariable* sSrvs;
	ConstantBuffer* pointLightBuffer;
	ID3DX11EffectConstantBuffer* sPointLightBuffer;

	ConstantBuffer* spotLightBuffer;
	ID3DX11EffectConstantBuffer* sSpotLightBuffer;

	Render2D* debug2D[6];
};
