#include <limits>
#define SIZEARR 1600

#include "Matrix.h"
#include "Quaternion.h"

float  RADIUS = 1.5 ;

float randBetween(int low, int high) {
   return float (((rand() % (high - low)) + low));
}



class StateVectorBall
{
	public:
	
	Vector3d position;
	
	Quaternion q;
	
	Vector3d momentum, angularMomentum ;
		
	StateVectorBall():position(-8,15,-5),q(),momentum(0,0,0),angularMomentum(0.0,0.0,0.0)
	{}
	
	
	StateVectorBall(const StateVectorBall & rhs)
	{
		position = rhs.position;
		
		q = rhs.q;
		
		momentum = rhs.momentum;
		
		angularMomentum = rhs.angularMomentum;
	}
	
	StateVectorBall& operator=(const StateVectorBall & rhs)
	{
		
		position = rhs.position;
		
		q = rhs.q;
		
		momentum = rhs.momentum;
		
		angularMomentum = rhs.angularMomentum;
		
		return *this;
	}
	
	//StateVectorBall operator*(const StateVectorBall& v, double s)
	StateVectorBall operator*(double s)
	{
		
		StateVectorBall temp;
		
		temp.position = s * position;
		
		temp.q = s * q;
		
		temp.momentum = s * momentum;
		
		temp.angularMomentum = s * angularMomentum;
		
		return temp;
	}
  	
	StateVectorBall operator+(const StateVectorBall& rhs)
	{
		//StateVectorBall temp(size);
		StateVectorBall temp;
		
		temp.position = position + rhs.position ;
		
		temp.q = q + rhs.q ;		
		
		temp.momentum = momentum + rhs.momentum ;		
		
		temp.angularMomentum = angularMomentum + rhs.angularMomentum ;		
		
		return temp;
	}
};



class RigidBody
{
	public:
	
	static int size;
	
	RigidBody():mass(25),Ibody(Matrix3x3(1,0,0,0,1,0,0,0,1)),R(Matrix3x3(1,0,0,0,1,0,0,0,1))
	{	
		Ibody = ( 0.4 * mass * RADIUS * RADIUS ) * Ibody ;
	
		Ibodyinv = Ibody.inv();
	}
	

	//Constant quantities
	
	float mass;
	
	Matrix3x3 Ibody, Ibodyinv ;
	
	
	//State variables
	
	StateVectorBall SV;
	
	
	//Derived quantities
	
		
	Matrix3x3 Iinv, R ;
	
	Vector3d omega;
	
	//Vector3d v, omega, force, torque;
	
};


/*
class RigidBody
{
	
	public: 
	
	RigidBody():Position(),Velocity(),Acceleration(),mass(25)
	{}
	
	Vector3d Position, Velocity, Acceleration;
	float mass;

};*/


class SpringVertex
{
	public:
	
		
	SpringVertex():mass(0.3),Position(0,0,0), Velocity(0,0,0) , Acceleration(0,0,0)
	{
							
		color[0] = 0.55;
		color[1] = 1;
		color[2] = 0;
				
	}
	
		
	float color[3];
	float mass;
	Vector3d Position;
	Vector3d Acceleration;
	Vector3d Velocity;
	
};



class StateVector
{
public:
	
	Vector3d arr[SIZEARR];
	int size;

		
	StateVector():size(SIZEARR)
	{}
	
	StateVector(const StateVector & rhs):size(rhs.size)
	{
		for(int i=0; i<rhs.size; i++)
		{
			arr[i] = rhs.arr[i] ;
		}
	}
	
	
	StateVector operator*(double s)
	{		
		StateVector temp;
		
		for(int i=0; i<size; i++)
		{
			temp.arr[i] = s * arr[i] ;
		}
		
		return temp;
	}
   
  	
	StateVector operator+(const StateVector& rhs)
	{		
		StateVector temp;
		
		for(int i=0; i<size; i++)
		{
			temp.arr[i] = arr[i] + rhs.arr[i] ;
		}		
		
		return temp;
	}
	
	StateVector& operator=(const StateVector& rhs)
	{			
		size = rhs.size ;
		
		for(int i=0; i<rhs.size; i++)
		{
			arr[i] = rhs.arr[i] ;
		}
		
		return *this;
	}

};



