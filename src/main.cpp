#include "MainWindow.h"

#include <windows.h>

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE,
                     PWSTR,
                     int nCmdShow) {
    InitCommonControls();
    MainWindow app;
    return app.run(hInstance, nCmdShow);
}
