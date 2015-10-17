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

	TMat4x4() {}
	TMat4x4(
		const T& _m00, const T& _m01, const T& _m02, const T& _m03,
		const T& _m10, const T& _m11, const T& _m12, const T& _m13,
		const T& _m20, const T& _m21, const T& _m22, const T& _m23,
		const T& _m30, const T& _m31, const T& _m32, const T& _m33
	):
		m00(_m00),m01(_m01),m02(_m02),m03(_m03),
		m10(_m10),m11(_m11),m12(_m12),m13(_m13),
		m20(_m20),m21(_m21),m22(_m22),m23(_m23),
		m30(_m30),m31(_m31),m32(_m32),m33(_m33)

	{
	}

	static TMat4x4 Zero()
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0);
	};

	static TMat4x4 Identity()
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1);
	};

	static TMat4x4 RotX(const T& s, const T& c)
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( 1, 0, 0, 0,  0, c,-s, 0,  0, s, c, 0,  0, 0, 0, 1);
	};

	static TMat4x4 RotY(const T& s, const T& c)
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( c, 0, s, 0,  0, 1, 0, 0, -s, 0, c, 0,  0, 0, 0, 1);
	};

	static TMat4x4 RotZ(const T& sina, const T& cosa)
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( c,-s, 0, 0,  s, c, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1);
	};

	static TMat4x4 ScaleNU(const T& x, const T& y, const T& z)
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( x, 0, 0, 0,  0, y, 0, 0,  0, 0, z, 0,  0, 0, 0, 1);
	};

	static TMat4x4 Scale(const T& s)
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( s, 0, 0, 0,  0, s, 0, 0,  0, 0, s, 0,  0, 0, 0, 1);
	};

	static TMat4x4 Translation(const T& x, const T& y, const T& z)
	{
		//             00 01 02 03  10 11 12 13  20 21 22 23  30 31 32 33
		return TMat4x4( 1, 0, 0, x,  0, 1, 0, y,  0, 0, 1, z,  0, 0, 0, 1);
	};

	// think of infinity projection matrix, zf=infinity
	// if zf==0, then we generate infinity projection matrix.
	static TMat4x4 Projection(T zn, T zf, T hfov, T vfov, T x0=-1, T x1=1, T y0=-1, T y1=1)
	{
		T dx=(x1-x0);
		T dy=(y1-y0);
		T xs;
		T ys;
		xs=1.0f/(hfov*0.5f);
		ys=1.0f/(vfov*0.5f);
		T q;
		if(zf==0)
			q=1;
		else
			q=zf/(zf-zn);

		TMat4x4 t=Zero();
		t.m[0][0]=2*xs/dx;
		t.m[1][1]=2*ys/dy;
		t.m[2][0]=-(2*x0+dx)/dx;
		t.m[2][1]=-(2*y0+dy)/dy;
		t.m[2][2]=q;
		t.m[2][3]=1;
		t.m[3][2]=-q*zn;

		return t;
	}

	static inline void Swp(T& x1, T& x2){T tmp=x1;x1=x2;x2=tmp;}
	void Transpose()
	{
		Swp(m01,m10);
		Swp(m02,m20);
		Swp(m03,m30);
		Swp(m12,m21);
		Swp(m13,m31);
		Swp(m23,m32);
	}
	TMat4x4<T> Transposed()
	{
		TMat4x4<T> t=*this;
		t.Transpose();
		return t;
	}

};

template <typename T> TMat4x4<T> operator* (const TMat4x4<T>& l, const TMat4x4<T>& r)
{
	int i, j, k;
	TMat4x4<T> t=TMat4x4<T>::Zero();
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			for(k=0; k<4; k++)
				t.m[i][j]+=l.m[i][k]*r.m[k][j];
	return t;
}

typedef TMat4x4<float> float4x4;


