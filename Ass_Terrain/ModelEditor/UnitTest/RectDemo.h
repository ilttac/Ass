#pragma once

#include "Systems/IExecute.h"

class RectDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {};
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {};
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	Shader * shader;

	VertexColor vertices[6];
	ID3D11Buffer* vertexBuffer;
};
