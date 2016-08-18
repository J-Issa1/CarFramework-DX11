#pragma once

#include "Transform.h"

class ParticleModel
{
public:
	ParticleModel(Transform * transform);
	virtual ~ParticleModel();

	void Move(float xAmount, float yAmount, float zAmount);

	void MoveConstantVelocity(float t);
	void MoveConstantAcceleration(float t);

	XMFLOAT3 GetVelocity() const { return _velocity; }
	void SetVelocity(XMFLOAT3 velocity) { _velocity = velocity; }

	XMFLOAT3 GetAcceleration() const { return _acceleration; }
	void SetAcceleration(XMFLOAT3 acceleration) { _acceleration = acceleration; }

	float GetMass() const { return _mass; }
	void SetMass(float mass) { _mass = mass; }

	float GetCollisionRadius() const { return _radius; }
	void SetCollisionRadius(float collisionRadius) { _radius = collisionRadius; }
	 
	void SetSlidingForce(bool slideBool);

	void CheckFloorCollision(XMFLOAT3 position);
	bool CheckCollision(XMFLOAT3 position, float radius);

	void ResolveCollision(ParticleModel* particleModel1, ParticleModel* particleModel2);

	void UpdateNetForce();
	void UpdateAccel();
	void UpdateState(float t);
	virtual void Update(float t);

	void SlidingForce(float theta, float frCoef);
	void SlidingMotion(float t);

	void MotionInFluid();

	void DragForce();
	void DragLamFlow();
	void DragTurbFlow();

	// AI
	void Move(XMFLOAT3 movement);
	XMFLOAT3 move;
	float moveBy;

private:	
	XMFLOAT3 _acceleration;

	bool _useConstAcc;
	bool laminar;
	bool slideOn;

	XMFLOAT3 netForce;
	XMFLOAT3 force;
	XMFLOAT3 slidingForce;
	XMFLOAT3 dragForce;

	float forceMag;
	float dragFactor;

	float _radius; 

protected:
	float gravity;
	float _mass;

	Transform* _transform;

	XMFLOAT3 _velocity;
};

