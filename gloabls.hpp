#pragma once
#include <Windows.h>

namespace sdk {
    DWORD process_id;
    DWORD64 module_base;
}

namespace config {
    inline bool show_menu = true;

    namespace aimbot {
        inline bool enable = false;

        inline int fov = 150;
        inline float smoothing = 2.f;
    }

    namespace visuals {
        inline bool enable = false;
        inline bool snapline = false;
    }
}