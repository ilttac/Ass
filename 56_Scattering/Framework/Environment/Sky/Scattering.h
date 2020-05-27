#pragma once

class Scattering
{
public:
	Scattering(Shader* shader);
	~Scattering();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

	void Pass(UINT val) { pass = val; }
	void SetDebug(bool val) { bDebug = val; }

private:
	void CreateQuad();

private:
	bool bDebug = false;
	UINT pass = 0;

	const UINT width, height;

	Shader* shader;
	Render2D* render2D;

	RenderTarget* mieTarget, *rayleighTarget;
	DepthStencil* depthStencil;
	Viewport* viewport;

	VertexBuffer* vertexBuffer;
};