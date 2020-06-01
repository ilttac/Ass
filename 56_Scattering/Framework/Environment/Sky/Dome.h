#pragma once

class Dome : public Renderer
{
public:
	Dome(Shader* shader, Vector3 position, Vector3 Scale, UINT drawCount = 32);
	~Dome();

	void Update() override;
	void Render() override;

private:
	UINT drawCount;

};