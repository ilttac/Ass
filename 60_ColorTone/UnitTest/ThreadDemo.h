#pragma once

#include "Systems/IExecute.h"

class ThreadDemo : public IExecute
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
	void Thread();

	void Thread2();
	void Thread2_1();
	void Thread2_2();

	void Join();

	void Mutex();

private:
	mutex m;
	float progress = 0;

	Timer timer;
	Timer timer2;
	
};
