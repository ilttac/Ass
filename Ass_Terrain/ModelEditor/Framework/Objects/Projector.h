#pragma once

class Projector
{
public:
	Projector(Shader* shader, wstring mapFile, UINT width, UINT height);
	~Projector();

	void Update();
	void Render();

private:
	struct Desc
	{
		Matrix View;
		Matrix Projection;

		Color color = D3DXCOLOR(1, 1, 1, 1);
	} desc;

private:
	Shader * shader;

	UINT width, height;
	class Fixity* camera;
	Projection* projection;


	Texture* map;
	ID3DX11EffectShaderResourceVariable* sMap;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;


};