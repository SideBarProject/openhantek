#pragma once

#ifdef DEBUG
    #include <iostream>
    #include <chrono>
    #include <ctime>
    #include <iomanip>
    #define timestampDebug(debug_string) {\
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();\
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);\
        std::cout << std::put_time(std::localtime(&now_c), "%T") << debug_string << std::endl;\
        }
#else
    #define timestampDebug(x)
#endif
