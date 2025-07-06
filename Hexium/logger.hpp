#pragma once
#include <stdio.h>
#include "platform.hpp"
#include "Utils/Notify.h"

// surely its not copied from random cs2 sdk
#define LOG(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_RED, fmt, __VA_ARGS__)
#define LOG_WARNING(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN, fmt, __VA_ARGS__)
#define LOG_SUCCESS(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_GREEN, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_BLUE, fmt, __VA_ARGS__)

namespace logger {
    inline void log(const char* file, int line, WORD color, const char* fmt, ...)
    {

        // Get current time
        SYSTEMTIME time;
        GetLocalTime(&time);

        // Save the current console attributes
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD originalAttrs = consoleInfo.wAttributes;

        // Print the time
        printf("[%02d:%02d:%02d] ", time.wHour, time.wMinute, time.wSecond);

        // remove path from file
        const char* file_name = strrchr(file, '\\') + 1;

        // Set the desired text color for file:line
        SetConsoleTextAttribute(hConsole, color);
        printf("%s:%d:", file_name, line);

        // Reset to original console attributes
        SetConsoleTextAttribute(hConsole, originalAttrs);
        printf(" ");

        // Format the log message using sprintf_s
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf_s(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        // Print the formatted message
        printf("%s\n", buffer);

#if _DEBUG
        // kinda joke 
        auto logstring = std::string(buffer);
        logstring = logstring.substr(0, logstring.find_last_of('\n'));
        logstring = logstring.substr(0, logstring.find_last_of('\r'));
        Notify::Send(logstring.c_str(), 2500);
#endif
    }
}
