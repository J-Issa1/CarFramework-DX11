#include "CarParticleModel.h"

CarParticleModel::CarParticleModel(Transform * transform) : ParticleModel(transform)
{
	RPM = 0.0f;
	wheelRadius = 3.0f;
	_engineSpeed = 0.0f;
	gearRatio = 6;

	_transform = transform;

	SetMass(1.0f);

	SetCollisionRadius(5.0f);
	SetSlidingForce(false);
}

CarParticleModel::~CarParticleModel()
{
}

void CarParticleModel::CalculateWheelSpeedRPM()
{
	RPM = _engineSpeed / gearRatio;
}

void CarParticleModel::CalculateForwardVelocity(float t)
{
	// Linear Speed = Arc Length / time
	//      v       =    AP      /  t
	thrust = RPM * wheelRadius * (2 * XM_PI) / t;

	float carRotation;
	carRotation = _transform->GetRotation().y;

	carForwardVector.x = sin(carRotation);
	carForwardVector.y = 0.0f;
	carForwardVector.z = cos(carRotation);

	float carForwardMagnitude = sqrt((carForwardVector.x * carForwardVector.x) + (carForwardVector.y * carForwardVector.y) + (carForwardVector.z * carForwardVector.z));

	carForwardVector = XMFLOAT3((carForwardVector.x / carForwardMagnitude), (carForwardVector.y / carForwardMagnitude), (carForwardVector.z / carForwardMagnitude));

	_velocity.x = carForwardVector.x * thrust;
	_velocity.z = carForwardVector.z * thrust;
}

void CarParticleModel::Update(float t)
{
	CalculateWheelSpeedRPM();
	CalculateForwardVelocity(t);
	//CalculateGear(t);

	//if (RPM < 0.0f)
	//	RPM = 0.0f;

	ParticleModel::Update(t);
}

/*void CarParticleModel::CalculateGear(float t)
{
	if (gearRatio == 6)
	{
		if (_engineSpeed >= 20.0f)
		{
			_engineSpeed = 20.0f;
		}
	}
	else if (gearRatio == 5)
	{
		if (_engineSpeed >= 30.0f)
		{
			_engineSpeed = 30.0f;
		}
	}
	else if (gearRatio == 4)
	{
		if (_engineSpeed >= 60.0f)
		{
			_engineSpeed = 60.0f;
		}
	}
}*/

