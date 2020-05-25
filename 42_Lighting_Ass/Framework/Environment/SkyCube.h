#pragma once

class SkyCube
{
public:
	SkyCube(wstring file);
	~SkyCube();

	void Update();
	void Render();

private:
	Shader * shader;
	MeshRender* sphere;

	ID3D11ShaderResourceView* srv;
	ID3DX11EffectShaderResourceVariable* sSrv;
};