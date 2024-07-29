#ifndef HILL_APPLICATION_H
#define HILL_APPLICATION_H

#include "DirectX11Application.h"

class HillApplication : public DirectX11Application
{
public:
	HillApplication(HINSTANCE hInstance);

	virtual void BuildGeometryBuffer() override;

	virtual void UpdateScene(float dt) override;

	float GetHeight(float x, float z) const;

protected:


private:

};
#endif // !HILL_APPLICATION_H
