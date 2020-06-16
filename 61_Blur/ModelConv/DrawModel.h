#pragma once

#include "Systems/IExecute.h"

class DrawModel : public IExecute
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
	void Airplane();
	void Tower();
	void Tank();

private:
	Shader * shader;

	ModelRender* airPlane = NULL;
	ModelRender* tower = NULL;
	ModelRender* tank = NULL;
};