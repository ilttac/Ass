#pragma once

#include "Systems/IExecute.h"

class DrawMesh : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() {}
	virtual void Destroy() {}
	virtual void Update()  override;
	virtual void PreRender() {}
	virtual void Render()  override;
	virtual void PostRender() {}
	virtual void ResizeScreen() {}


private:
	Shader * shader;

	Material* gridMaterial;
	MeshRender* grid;

	Material* cubeMaterial;
	MeshRender* cube;

};