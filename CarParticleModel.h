#pragma once
#include "ParticleModel.h"

class CarParticleModel : public ParticleModel
{
public:
	CarParticleModel(Transform * transform);
	~CarParticleModel();

	float GetRPM() { return RPM; }
	void SetRPM(float rpm) { RPM = rpm; }

	float GetEngineSpeed() { return _engineSpeed; }
	void SetEngineSpeed(float engineSpeed) { _engineSpeed = engineSpeed; }

	int GetGear() { return gearRatio; }
	void SetGear(int gear) { gearRatio = gear; }

	void CalculateWheelSpeedRPM();
	void AddEngineSpeed(float engineSpeed) { _engineSpeed += engineSpeed; }
	void CalculateForwardVelocity(float t);

	void CalculateGear(float t);

	XMFLOAT3 GetCarForwardVector(){ return carForwardVector; }

	void Update(float t);

private:
	float wheelRadius;
	float RPM;
	float _engineSpeed;
	float thrust;
	int gearRatio;
	float frictionCoef;
	float steeringAngle;

	XMFLOAT3 carForwardVector;
};

