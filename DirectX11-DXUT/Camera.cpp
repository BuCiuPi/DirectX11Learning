#include "Camera.h"

XMVECTOR Camera::GetViewDirection()
{
	return XMVector3Normalize(Target - Position);
}
