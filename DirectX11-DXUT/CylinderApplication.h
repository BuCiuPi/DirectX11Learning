#ifndef CYLINDER_APPLICATION_H
#define CYLINDER_APPLICATION_H

#include "DirectX11Application.h"

class CylinderApplication : public DirectX11Application
{
public:
	CylinderApplication(HINSTANCE hInstance);

	virtual void BuildGeometryBuffer() override;
	virtual void UpdateScene(float dt) override;

private:

};

#endif // !CYLINDER_APPLICATION_H
