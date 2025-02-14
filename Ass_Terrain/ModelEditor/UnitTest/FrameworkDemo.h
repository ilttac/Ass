#pragma once

#include "Systems/IExecute.h"

class FrameworkDemo : public IExecute
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
	
	MeshRender* grid;
	MeshRender* cube;
	MeshRender* cylider;
	MeshRender* sphere;

	Shader* terrainShader;
	class Terrain* terrain;
	
};
