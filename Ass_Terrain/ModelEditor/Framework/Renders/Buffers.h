#pragma once

class VertexBuffer
{
public:
	VertexBuffer(void* data, UINT count, UINT stride, UINT slot = 0, bool bCpuWrite = false, bool bGpuWrite = false);
	~VertexBuffer();

	UINT Count() { return count; }
	UINT Stride() { return stride; }
	ID3D11Buffer* Buffer() { return buffer; }

	void Render();

private:
	ID3D11Buffer * buffer;

	void* data;
	UINT count;
	UINT stride;
	UINT slot;

	bool bCpuWrite;
	bool bGpuWrite;
};

////////////////////////////////////////////////////////////////////////////////////

class IndexBuffer
{
public:
	IndexBuffer(void* data, UINT count);
	~IndexBuffer();

	UINT Count() { return count; }
	ID3D11Buffer* Buffer() { return buffer; }

	void Render();

private:
	ID3D11Buffer * buffer;

	void* data;
	UINT count;
};

////////////////////////////////////////////////////////////////////////////////////

class ConstantBuffer
{
public:
	ConstantBuffer(void* data, UINT dataSize);
	~ConstantBuffer();

	ID3D11Buffer* Buffer() { return buffer; }

	void Apply(); //변경된 데이터를 갱신하는 함수

private:
	ID3D11Buffer * buffer;

	void* data;
	UINT dataSize;
};

////////////////////////////////////////////////////////////////////////////////////
class CsResource
{
public:
	CsResource();
	virtual ~CsResource();

	ID3D11ShaderResourceView* SRV() { return srv; }
	ID3D11UnorderedAccessView* UAV() { return uav; }

protected:
	virtual void CreateInput() {}
	virtual void CreateSRV() {}

	virtual void CreateOutput() {}
	virtual void CreateUAV() {}

	virtual void CreateResult() {}

	void CreateBuffer();

protected:
	ID3D11Resource * input = NULL;
	ID3D11ShaderResourceView* srv = NULL;

	ID3D11Resource* output = NULL;
	ID3D11UnorderedAccessView* uav = NULL;

	ID3D11Resource* result = NULL;
};

////////////////////////////////////////////////////////////////////////////////////
class RawBuffer : public CsResource
{
public:
	RawBuffer(void* inputData, UINT byteWidth);
	~RawBuffer();

	UINT ByteWidth() { return byteWidth; }

	void Copy(void* data, UINT size);

private:
	void CreateInput() override;
	void CreateSRV() override;

	void CreateOutput() override;
	void CreateUAV() override;

	void CreateResult() override;

private:
	void* inputData;
	UINT byteWidth;
};

////////////////////////////////////////////////////////////////////////////////////
class StructuredBuffer : public CsResource
{
public:
	StructuredBuffer(void *inputData, UINT inputStride, UINT inputCount, UINT outputStride = 0, UINT outputCount = 0);
	~StructuredBuffer();

	UINT InputByteWidth() { return inputStride * inputCount; }
	UINT OutputByteWidth() { return outputStride * outputCount; }

	void Copy(void* data, UINT size);
	void Map(void* data, UINT size);

private:
	void CreateInput() override;
	void CreateSRV() override;
	
	void CreateOutput() override;
	void CreateUAV() override;

	void CreateResult() override;

private:
	void* inputData;

	UINT inputStride;
	UINT inputCount;

	UINT outputStride;
	UINT outputCount;

};