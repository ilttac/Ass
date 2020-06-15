#pragma once

class PostEffect : public Renderer
{
public:
	PostEffect(wstring shaderFile);
	~PostEffect();

	void Update();
	void Render();

	void SRV(ID3D11ShaderResourceView* srv);

private:
	ID3DX11EffectShaderResourceVariable* sDiffuseMap;

};