#ifndef BOX_APLLICATION_H
#define BOX_APLLICATION_H

#include "DirectX11Application.h"
class BoxApplication : public DirectX11Application
{
public:
	BoxApplication(HINSTANCE hInstance);

	virtual void DrawScene() override;

	virtual void BuildGeometryBuffer() override;
private:

};

#endif // !BOX_APLLICATION_H

