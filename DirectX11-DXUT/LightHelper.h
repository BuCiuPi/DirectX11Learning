#ifndef LIGHT_HELPER_H
#define LIGHT_HELPER_H

#include <Windows.h>
#include "DirectXMath.h"

using namespace DirectX;

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT3 Direction;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	XMFLOAT3 Position;
	float Range;

	XMFLOAT3 Attenuation;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	XMFLOAT3 Position;
	float Spot;

	XMFLOAT3 Direction;
	float Range;

	XMFLOAT3 Attenuation;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct CapsuleLight
{
	CapsuleLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Diffuse;

	XMFLOAT3 Position;
	float Range;

	XMFLOAT3 Direction;
	float Len;
};

struct FourCapsuleLight
{
	XMFLOAT4 DiffuseR;
	XMFLOAT4 DiffuseG;
	XMFLOAT4 DiffuseB;

	XMFLOAT4 PositionX;
	XMFLOAT4 PositionY;
	XMFLOAT4 PositionZ;

	XMFLOAT4 Range;

	XMFLOAT4 DirectionX;
	XMFLOAT4 DirectionY;
	XMFLOAT4 DirectionZ;

	XMFLOAT4 Len;
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w= SpecPower
	XMFLOAT4 Reflect;
};

#endif // !LIGHT_HELPER_H
