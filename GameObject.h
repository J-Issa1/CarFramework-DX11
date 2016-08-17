#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>

#include "Appearance.h"
#include "Transform.h"
#include "ParticleModel.h"

using namespace DirectX;
using namespace std;

class GameObject
{
public:
	GameObject(string type, Appearance * appearance, Transform * transform, ParticleModel * particleModel);
	~GameObject();

	string GetType() const { return _type; }

	Appearance * GetAppearance() const { return _appearance; }
	Transform * GetTransform() const { return _transform; }
	ParticleModel * GetParticleModel() const { return _particleModel; }

	void Update(float t);
	void Draw(ID3D11DeviceContext * pImmediateContext);

private:
	string _type;

	Appearance * _appearance;
	Transform * _transform;
	ParticleModel * _particleModel;
};

