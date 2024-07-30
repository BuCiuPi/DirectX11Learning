#ifndef SPHERE_APPLICATION_H
#define SPHERE_APPLICATION_H

#include "DirectX11Application.h"

class SphereApplication : public DirectX11Application
{
public:
	SphereApplication(HINSTANCE hInstance);

	virtual void BuildGeometryBuffer() override;
	virtual void UpdateScene(float dt) override;


private:

};

#endif // !SPHERE_APPLICATION_H
