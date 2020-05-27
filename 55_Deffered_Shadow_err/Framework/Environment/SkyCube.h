#pragma once

class SkyCube
{
public:
	SkyCube(wstring file, Shader* shader = NULL);
	~SkyCube();

	void Update();
	void Render();

	void Pass(UINT val) { pass = val; }

private:
	UINT pass = 0;

	bool bCreateShader = false;
	Shader * shader;
	MeshRender* sphereRender;

	ID3D11ShaderResourceView* srv;
	ID3DX11EffectShaderResourceVariable* sSrv;

	ID3D11DepthStencilState* dss;
	ID3DX11EffectDepthStencilVariable* sDss;
};