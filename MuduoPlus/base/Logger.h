#pragma once

#include <functional>

namespace MuduoPlus
{

    enum LogType
    {
        LogType_Info,
        LogType_Debug,
        LogType_Warn,
        LogType_Error,
        LogType_Fatal,
    };

    typedef void(*LogPrintFunc)(LogType type, const char *format, ...);
    extern  LogPrintFunc LogPrinter;

#define LOG_PRINT(type, format, ...)    if (LogPrinter) LogPrinter(type, format, ##__VA_ARGS__)
}