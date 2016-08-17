#pragma once

#include <directxmath.h>

using namespace DirectX;

class Transform
{
public:
	Transform();
	~Transform();

	void SetPosition(XMFLOAT3 position) { oldPosition = _position; _position = position; }
	void SetPosition(float x, float y, float z) { SetPosition(XMFLOAT3(x, y, z)); }
	
	XMFLOAT3 GetPosition() const { return _position; }

	XMFLOAT3 GetOldPosition() const { return oldPosition; }

	void SetScale(XMFLOAT3 scale) { _scale = scale; }
	void SetScale(float x, float y, float z) { _scale.x = x; _scale.y = y; _scale.z = z; }

	XMFLOAT3 GetScale() const { return _scale; }

	void SetRotation(XMFLOAT3 rotation) { _rotation = rotation; }
	void SetRotation(float x, float y, float z) { _rotation.x = x; _rotation.y = y; _rotation.z = z; }

	XMFLOAT3 GetRotation() const { return _rotation; }

	XMMATRIX GetWorldMatrix() const { return XMLoadFloat4x4(&_world); }

	void SetParent(Transform * parent) { _parent = parent; }

	void Update(float t);

private:
	XMFLOAT3 oldPosition;
	XMFLOAT3 _position;
	XMFLOAT3 _rotation;
	XMFLOAT3 _scale;

	XMFLOAT4X4 _world;

	Transform * _parent;
};

