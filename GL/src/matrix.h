#pragma once

template <typename T> union TMat4x4
{
	struct{
		T m00;T m01;T m02;T m03;
		T m10;T m11;T m12;T m13;
		T m20;T m21;T m22;T m23;
		T m30;T m31;T m32;T m33;
	};
	T m[4][4];
	T m16[16];

	void Zero()
	{
		m00=0.0f;m01=0.0f;m02=0.0f;m03=0.0f;
		m10=0.0f;m11=0.0f;m12=0.0f;m13=0.0f;
		m20=0.0f;m21=0.0f;m22=0.0f;m23=0.0f;
		m30=0.0f;m31=0.0f;m32=0.0f;m33=0.0f;
	};

	void Identity()
	{
		m00=1.0f;m01=0.0f;m02=0.0f;m03=0.0f;
		m10=0.0f;m11=1.0f;m12=0.0f;m13=0.0f;
		m20=0.0f;m21=0.0f;m22=1.0f;m23=0.0f;
		m30=0.0f;m31=0.0f;m32=0.0f;m33=1.0f;
	};

	void RotationX(const T& sina, const T& cosa)
	{
		Identity();
		m[1][1]=cosa;
		m[1][2]=-sina;
		m[2][1]=sina;
		m[2][2]=cosa;
	};

	void RotationY(const T& sina, const T& cosa)
	{
		Identity();
		m[0][0]=cosa;
		m[0][2]=sina;
		m[2][0]=-sina;
		m[2][2]=cosa;
	};

	void RotationZ(const T& sina, const T& cosa)
	{
		Identity();
		m[0][0]=cosa;
		m[0][1]=-sina;
		m[1][0]=sina;
		m[1][1]=cosa;
	};
};

typedef TMat4x4<float> float4x4;


