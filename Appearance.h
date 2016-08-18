#pragma once

#include "Structures.h"
#include <directxmath.h>
#include <d3d11_1.h>

using namespace DirectX;

class Appearance
{
public:
	Appearance(Geometry geometry, Material material);
	~Appearance();

	Geometry GetGeometryData() const { return _geometry; }

	Material GetMaterial() const { return _material; }

	ID3D11ShaderResourceView* GetTextureRV() const { return _textureRV; }
	void SetTextureRV(ID3D11ShaderResourceView* textureRV) { _textureRV = textureRV; }
	bool HasTexture() const { return _textureRV ? true : false; }

	void Draw(ID3D11DeviceContext * pImmediateContext);

private:
	Geometry _geometry;
	Material _material;

	ID3D11ShaderResourceView * _textureRV;
};

