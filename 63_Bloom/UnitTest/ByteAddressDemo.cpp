#include "stdafx.h"
#include "ByteAddressDemo.h"

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

void ByteAddressDemo::Initialize()
{
	Shader* shader = new Shader(L"34_ByteAddress.fx");

	struct Output
	{
		UINT GroupID[3];
		UINT GroupThreadID[3];
		UINT DispatchThreadID[3];
		UINT GroupIndex;
	};

	UINT size = 3 * 10 * 8 * 3;

	Output* output = new Output[size];
	RawBuffer* buffer = new RawBuffer(NULL, sizeof(Output) * size);

	shader->AsUAV("Output")->SetUnorderedAccessView(buffer->UAV());

	shader->Dispatch(0, 0, 3, 1, 1);

	buffer->Copy(output, sizeof(Output) * size);

	FILE* file;
	fopen_s(&file, "../ByteAddress.csv", "w");

	for (UINT i = 0; i < size; i++)
	{
		Output temp = output[i];

		fprintf
		(
			file,
			"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			i,
			temp.GroupID[0],
			temp.GroupID[1],
			temp.GroupID[2],
			temp.GroupThreadID[0],
			temp.GroupThreadID[1],
			temp.GroupThreadID[2],
			temp.DispatchThreadID[0],
			temp.DispatchThreadID[1],
			temp.DispatchThreadID[2],
			temp.GroupIndex
		);
	}
	fclose(file);

}

void ByteAddressDemo::Destroy()
{
}

void ByteAddressDemo::Update()
{
	
}

void ByteAddressDemo::Render()
{
	

}
