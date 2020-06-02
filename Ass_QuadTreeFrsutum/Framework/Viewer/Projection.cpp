#include "Framework.h"
#include "Projection.h"

Projection::Projection()
	: width(0), height(0), zn(0), zf(0), fov(0)
{
	D3DXMatrixIdentity(&matrix);
}

Projection::Projection(float width, float height, float zn, float zf, float fov)
	: width(width), height(height), zn(zn), zf(zf), fov(fov)
{

}

Projection::~Projection()
{

}

void Projection::GetMatrix(Matrix * matrix)
{
	memcpy(matrix, &this->matrix, sizeof(Matrix));
}

float Projection::GetWidth()
{
	return this->width;
}

float Projection::GetHeight()
{
	return this->height;
}

float Projection::GetZn()
{
	return this->zn;
}

float Projection::GetZf()
{
	return this->zf;
}

float Projection::GetFov()
{
	return this->fov;
}


void Projection::Set(float width, float height, float zn, float zf, float fov)
{
	this->width = width;
	this->height = height;
	this->zn = zn;
	this->zf = zf;
	this->fov = fov;
}
