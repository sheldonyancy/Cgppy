/**
 * MIT License
 *
 * Copyright (c) 2024 Sheldon Yancy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "YLogger.h"
#include "YDeveloperConsole.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <cstdio>

static std::shared_ptr<spdlog::logger> g_logger = nullptr;

void yLogInit(const i8* exe_path) {
    std::string log_file = std::string(exe_path) + "/log.txt";
    g_logger = spdlog::basic_logger_mt("basic_logger", log_file.c_str());
    g_logger->set_level(spdlog::level::info);
}

extern "C" i32 yStringFormatV(i8* dest, const i8* format, void* va_listp) {
    if (dest) {
        i8 buffer[8192];
        i32 written = std::vsnprintf(buffer, 8192, format, (i8*)va_listp);
        buffer[written] = 0;
        memcpy(dest, buffer, written + 1);

        return written;
    }
    return -1;
}

extern "C" i32 yStringFormat(i8* dest, const i8* format, ...) {
    if (dest) {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = yStringFormatV(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

extern "C" void yLogOutput(YeLogLevel level, const i8* message, ...)
{
#ifdef NDEBUG
    if( YeLogLevel::LOG_LEVEL_DEBUG == level) {
        return;
    }
#endif

    i8 out_message[8192];
    memset(out_message, 0, sizeof(out_message));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef i8* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    yStringFormatV(out_message, message, arg_ptr);
    va_end(arg_ptr);

    // Prepend log level to message.
    yStringFormat(out_message, "%s\n", out_message);

    g_logger->info(out_message);
    g_logger->flush();

    YDeveloperConsole::instance()->addLogMessage(level, out_message);
}

void yCReportAssertionFailure(const i8* expression, const i8* message, const i8* file, i32 line) {
    YFATAL("Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}