#include "Application.h"
#include <time.h>

double clockToMilliseconds(clock_t ticks) 
{
	return (ticks / (double)CLOCKS_PER_SEC)*1000.0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Time Measurement
	float frameProcessingTime = GetCurrentTime();
	float ElapsedTime = 0;
	float timeStep = 15.0f;
	clock_t deltaTime = 0;
	float frameRate = 30;
	unsigned int frames = 0;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Application* theApp = new Application();

	if (FAILED(theApp->Initialise(hInstance, nCmdShow)))
	{
		return -1;
	}

	// Main message loop
	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			bool handled = false;

			if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
			{
				handled = theApp->HandleKeyboard(msg, timeStep);
			}
			else if (WM_QUIT == msg.message)
				break;

			if (!handled)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		
		clock_t frameStartTime = clock();

		theApp->Update(timeStep);
		theApp->Draw();

		float frameEndTime = clock();
		ElapsedTime = frameEndTime - frameStartTime;
		if (ElapsedTime < timeStep)
		{
			Sleep(timeStep - ElapsedTime);
		}

		deltaTime += frameEndTime - frameStartTime;
		frames++;

		if (clockToMilliseconds(deltaTime)>133.8688085676037)
		{
			frameRate = frames*0.5 + frameRate*0.5;
			frames = 0;
			deltaTime = 0.0f;
		}
		
	}

	delete theApp;
	theApp = nullptr;

	return (int)msg.wParam;
}