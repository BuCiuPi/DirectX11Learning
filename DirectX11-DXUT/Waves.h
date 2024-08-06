#ifndef WAVES_H
#define WAVES_H

#include "Windows.h"
#include "DirectXMath.h"
#include "D3DUtil.h"

class Waves
{
public:
	Waves();
	~Waves();

	void ReleasePointer();
	
	UINT RowCount() const;
	UINT ColumnCount() const;
	UINT VertexCount() const;
	UINT TriangleCount() const;

	const XMFLOAT3& operator[](int i)const { return mCurrSolution[i]; }

	const XMFLOAT3& Normal(int i)const { return mNormals[i]; }

	const XMFLOAT3& TangentX(int i)const { return mTangentX[i]; }

	float Width() const;
	float Depth() const;


	void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
	void Update(float dt);
	void Disturb(UINT i, UINT j, float magnitude);

private:
	UINT mNumRows;
	UINT mNumCols;

	UINT mVertexCount;
	UINT mTriangleCount;

	// Simulation constants we can precompute.
	float mK1;
	float mK2;
	float mK3;

	float mTimeStep;
	float mSpatialStep;

	XMFLOAT3* mPrevSolution;
	XMFLOAT3* mCurrSolution;
	XMFLOAT3* mNormals;
	XMFLOAT3* mTangentX;

};

#endif // !WAVES_H
