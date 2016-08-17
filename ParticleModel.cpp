#include "ParticleModel.h"

ParticleModel::ParticleModel(Transform * transform) : _transform(transform)
{
	_transform = transform;

	gravity = -9.81f;
	_mass = 1.0f;

	_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Swap between velocity
	_useConstAcc = true;
	laminar = true;
	slideOn = false;

	force = XMFLOAT3(0.0f, 0.0f, 0.0f);
	netForce = XMFLOAT3(0.0f, 0.0f, 0.0f);

	forceMag = 0.0f;
	dragFactor = 1.0f;

	_radius = 0.5f;
}

ParticleModel::~ParticleModel()
{
}

void ParticleModel::Move(float xAmount, float yAmount, float zAmount)
{
	XMFLOAT3 position = _transform->GetPosition();

	position.x += xAmount;
	position.y += yAmount;
	position.z += zAmount;

	_transform->SetPosition(position);
}

// AI Move
void ParticleModel::Move(XMFLOAT3 move)
{
	XMFLOAT3 position = _transform->GetPosition();
	position.x += move.x;
	position.y += move.y;
	position.z += move.z;
	_transform->SetPosition(position);
}

void ParticleModel::MoveConstantVelocity(float t)
{
	t = t / 1000;

	XMFLOAT3 position = _transform->GetPosition();

	position.x += _velocity.x * t;
	position.y += _velocity.y * t;
	position.z += _velocity.z * t;

	_transform->SetPosition(position);
}

void ParticleModel::MoveConstantAcceleration(float t)
{
	t = t / 1000;

	XMFLOAT3 position = _transform->GetPosition();

	// S = UT + 0.5AT^2
	position.x += _velocity.x * t + 0.5f * _acceleration.x * t * t;
	position.y += _velocity.y * t + 0.5f * _acceleration.y * t * t;
	position.z += _velocity.z * t + 0.5f * _acceleration.z * t * t;

	// V = U + AT
	_velocity.x += _acceleration.x * t;
	_velocity.y += _acceleration.y * t;
	_velocity.z += _acceleration.z * t;

	_transform->SetPosition(position);
}

void ParticleModel::UpdateNetForce()
{
	netForce.x = dragForce.x + slidingForce.x;
	netForce.y = dragForce.y + slidingForce.y;
	netForce.z = dragForce.z + slidingForce.z;
}

void ParticleModel::UpdateAccel()
{
	_acceleration.x = netForce.x / _mass;
	_acceleration.y = netForce.y / _mass;
	_acceleration.z = netForce.z / _mass;
}

void ParticleModel::UpdateState(float t)
{
	UpdateNetForce();

	UpdateAccel();

	MoveConstantAcceleration(t);
}

void ParticleModel::Update(float t)
{
	if (_useConstAcc)
	{
		if (slideOn == true)
		{
			SlidingForce(XMConvertToRadians(90.0f), 10.0f);
		}
		else
		{
			slidingForce.x = 0.0f;
			slidingForce.y = _mass * gravity;
			slidingForce.z = 0.0f;
		}
		
		DragForce();

		UpdateState(t);
	}
	else
	{
		MoveConstantVelocity(t);
	}
}

void ParticleModel::SlidingForce(float theta, float frCoef)
{
	// calculate magnitude of force
	forceMag = _mass * gravity * (sin(theta) - frCoef * cos(theta));

	// calculate x- and y-components of force (note: x-axis assumed positive rightwards and y-axis positive downwards)
	slidingForce.x = forceMag * cos(theta);
	slidingForce.y = forceMag * sin(theta);
	slidingForce.z = forceMag * cos(theta);

	if (slidingForce.y > 0.0f)
	{
		slidingForce.x = 0.0f;
		slidingForce.y = 0.0f;
		//slidingForce.z = 0.0f;
	}

	// TODO: Check to ensure that magnitude is not negative
	//if (forceMag <= 0.0f)
	//	forceMag = -forceMag;
}

void ParticleModel::SetSlidingForce(bool slideBool)
{
	slideOn = slideBool;
}

void ParticleModel::SlidingMotion(float t)
{
	//SlidingForce(XMConvertToRadians(25.0f), 0.2f);

	UpdateState(t);

	MoveConstantAcceleration(t);
}

void ParticleModel::MotionInFluid()
{
	DragForce();

	//UpdateState();
	//
	//Move();
}

void ParticleModel::DragForce()
{
	if (laminar)
		// calculate drag force for laminar flow
		DragLamFlow();
	else
		// calculate drag force for turbulent flow
		DragTurbFlow();

}

void ParticleModel::DragLamFlow()
{
	// calculate of x- and y-components of drag force
	dragForce.x = -dragFactor * _velocity.x;
	dragForce.y = -dragFactor * _velocity.y;
	dragForce.z = -dragFactor * _velocity.z;
}

void ParticleModel::DragTurbFlow()
{
	// calculate magnitude of velocity
	float velMag;
	velMag = sqrt((_velocity.x * _velocity.x) + (_velocity.y * _velocity.y) + (_velocity.z * _velocity.z));

	// calculate unit vector/normalised vector of velocity
	XMFLOAT3 unitVel;
	unitVel.x = (_velocity.x / velMag);
	unitVel.y = (_velocity.y / velMag);
	unitVel.z = (_velocity.z / velMag);

	// calculate magnitude of drag force
	float dragMag;
	dragMag = dragFactor * velMag * velMag;

	// calculate of x- and y-components of drag force
	dragForce.x = -dragMag * unitVel.x;
	dragForce.y = -dragMag * unitVel.y;
	dragForce.z = -dragMag * unitVel.z;

}

bool ParticleModel::CheckCollision(XMFLOAT3 position2, float radius2)
{
	XMFLOAT3 position = _transform->GetPosition();

	XMFLOAT3 difference;
	difference.x = position.x - position2.x;
	difference.y = position.y - position2.y;
	difference.z = position.z - position2.z;

	float differenceMag = sqrt((difference.x * difference.x) + (difference.y * difference.y) + (difference.z * difference.z));

	float rDistance = _radius + radius2;

	if (differenceMag <= rDistance)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ParticleModel::CheckFloorCollision(XMFLOAT3 floorPos)
{
	XMFLOAT3 objectPosition = _transform->GetPosition();

	XMFLOAT3 difference;
	difference.y = objectPosition.y - floorPos.y;

	float differenceMag = sqrt(difference.y * difference.y);

	if (differenceMag <= _radius)
	{
		slideOn = true;
		_velocity.y = -_velocity.y;
	}
	else
	{
		slideOn = false;
	}
}

void ParticleModel::ResolveCollision(ParticleModel* particleModel1, ParticleModel* particleModel2)
{
	float rCoef = 0.8f;

	XMFLOAT3 prevPos1 = particleModel1->_transform->GetOldPosition();
	XMFLOAT3 prevPos2 = particleModel2->_transform->GetOldPosition();

	float mass1 = particleModel1->GetMass();
	float mass2 = particleModel2->GetMass();

	XMFLOAT3 initialVelocity1 = particleModel1->GetVelocity();
	XMFLOAT3 initialVelocity2 = particleModel2->GetVelocity();

	particleModel1->_transform->SetPosition(prevPos1);
	particleModel2->_transform->SetPosition(prevPos2);

	XMFLOAT3 velocity1;
	XMFLOAT3 velocity2;

	velocity1.x = (mass1 * initialVelocity1.x) + (mass2 * initialVelocity2.x) + ((mass2 * rCoef) * (initialVelocity2.x - initialVelocity1.x)) / (mass1 + mass2);
	velocity1.y = (mass1 * initialVelocity1.y) + (mass2 * initialVelocity2.y) + ((mass2 * rCoef) * (initialVelocity2.y - initialVelocity1.y)) / (mass1 + mass2);
	velocity1.z = (mass1 * initialVelocity1.z) + (mass2 * initialVelocity2.z) + ((mass2 * rCoef) * (initialVelocity2.z - initialVelocity1.z)) / (mass1 + mass2);

	velocity2.x = (mass1 * initialVelocity1.x) + (mass2 * initialVelocity2.x) + ((mass1 * rCoef) * (initialVelocity1.x - initialVelocity2.x)) / (mass1 + mass2);
	velocity2.y = (mass1 * initialVelocity1.y) + (mass2 * initialVelocity2.y) + ((mass1 * rCoef) * (initialVelocity1.y - initialVelocity2.y)) / (mass1 + mass2);
	velocity2.z = (mass1 * initialVelocity1.z) + (mass2 * initialVelocity2.z) + ((mass1 * rCoef) * (initialVelocity1.z - initialVelocity2.z)) / (mass1 + mass2);
	
	particleModel1->SetVelocity(velocity1);
	particleModel2->SetVelocity(velocity2);
} 