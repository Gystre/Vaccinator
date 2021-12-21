#pragma once

#include "Window.h"

namespace ctrl
{

    class Control : public Window
    {
    public:
        Control(HWND hwnd = NULL)
            : Window(hwnd) {  }
    };

}