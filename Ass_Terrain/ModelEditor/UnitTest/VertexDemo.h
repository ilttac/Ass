#pragma once

#include "Systems/IExecute.h"

class VertexDemo : public IExecute
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

	Vertex vertices[3];
	ID3D11Buffer* vertexBuffer;
};
