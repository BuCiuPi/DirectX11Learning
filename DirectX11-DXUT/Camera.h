#ifndef CAMERA_H
#define CAMERA_H

#include "DirectXMath.h"
using namespace DirectX;

class Camera
{
public:
	Camera(XMFLOAT3 Pos) {
		mPosition = Pos;
		mUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
		mRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
		mLook = XMFLOAT3(0.0f, 0.0f, 1.0f);


		SetLens(XM_PIDIV2,1, 1.0f, 1000.0f);
	};

	XMVECTOR GetPositionXM() const;
	XMFLOAT3 GetPosition() const;

	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& value);

	XMVECTOR GetRightXM() const;
	XMFLOAT3 GetRight() const;
	XMVECTOR GetUpXM() const;
	XMFLOAT3 GetUp() const;
	XMVECTOR GetLookXM() const;
	XMFLOAT3 GetLook() const;

	float GetNearZ() const;
	float GetFarZ() const;
	float GetAspect() const;
	float GetFovY() const;
	float GetFovX() const;

	float GetNearWindowWidth() const;
	float GetNearWindowHeight() const;
	float GetFarWindowWidth() const;
	float GetFarWindowHeight() const;

	void SetLens(float fovY, float aspect, float near, float far);

	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	XMMATRIX View() const;
	XMMATRIX Proj() const;
	XMMATRIX ViewProj() const;

	void Strafe(float d);
	void Walk(float d);
	void Fly(float d);

	void Pitch(float angle);
	void RotateY(float angle);

	void UpdateViewMatrix();

	float CameraSpeed = 10.0f;

private:
	XMFLOAT3 mPosition;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;

	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

public:
};

#endif // !CAMERA_H
