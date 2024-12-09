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

#ifndef CGPPY_YLOGGER_H
#define CGPPY_YLOGGER_H

#include "YDefines.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>


typedef enum YeLogLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} YeLogLevel;

#ifdef __cplusplus
extern "C" {
#endif

void yLogInit(const i8* exe_path);

void yLogOutput(YeLogLevel level, const i8* message, ...);

void yCReportAssertionFailure(const i8* expression, const i8* message, const i8* file, i32 line);


#ifdef __cplusplus
}
#endif

#define YFATAL(message, ...) yLogOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#define YERROR(message, ...) yLogOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#define YWARN(message, ...)  yLogOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#define YINFO(message, ...)  yLogOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#define YDEBUG(message, ...) yLogOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#define YTRACE(message, ...) yLogOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);


#endif //CGPPY_YLOGGER_H
