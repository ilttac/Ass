#pragma once

class Moon : public Renderer
{
public:
	Moon(Shader* shader);
	~Moon();

	void Update();
	void Render(float theta);

	Texture* GetMoonTexture(){ return moon; }
	Texture* GetMoonGlowTexture() { return moonGlow; }

	float& GetDistance() { return distance; }
	float& GetGlowDistance () { return glowDistance; }

private:
	float GetAlpha(float theta);
	
	Matrix GetTransform(float theta);
	Matrix GetGlowTransform(float theta);
	
	

private:
	float distance, glowDistance;

	ID3DX11EffectScalarVariable* sAlpha;

	Texture* moon;
	Texture* moonGlow;
	ID3DX11EffectShaderResourceVariable* sMoon;

};