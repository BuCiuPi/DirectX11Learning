#ifndef MAINWIN_H
#define MAINWIN_H

#include "DirectXApp.h"

class MainWindow : public DirectXApp
{
public:
	MainWindow(HINSTANCE hInstance);

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
private:

};

#endif // !MAINWIN_H
