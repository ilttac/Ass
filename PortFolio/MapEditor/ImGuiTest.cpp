#include "Framework.h"
#include "ImGuiTest.h"

void ImGuiTest::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	Context::Get()->GetCamera()->Position(132, 42, -17);
	((Freedom*)Context::Get()->GetCamera())->Speed(100, 2);
}

void ImGuiTest::Destroy()
{
}

void ImGuiTest::Update()
{
	
}

void ImGuiTest::Render()
{
}
