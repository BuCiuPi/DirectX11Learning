#ifndef CAMERA_H
#define CAMERA_H

#include "DirectXMath.h"
using namespace DirectX;

class Camera
{
public:
	Camera(XMVECTOR Pos) {
		Position = Pos;
		Target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	};

	XMVECTOR GetViewDirection();
	
	XMVECTOR Position;
	XMVECTOR Target;
	XMVECTOR Up;
private:

};

#endif // !CAMERA_H
