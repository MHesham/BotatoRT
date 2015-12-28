//
// Copyright 2015 Muhamad Lotfy
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <Windows.h>
#include <crtdbg.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>

#define SAFE_CLOSE(X)	if (X) { CloseHandle(X); (X) = NULL; }
#define ROUNDUP(X, M)	((((X) / (M)) * (M)) + ((X) % (M) > 0 ? (M) : 0))

namespace Botato {

    class Logger
    {
    public:
        static void DbgPrintf(const wchar_t* pTxtFormat, ...)
        {
            const size_t LogBufferMax = 2048;
            wchar_t buffer[LogBufferMax] = { 0 };

            va_list formatArgs;
            va_start(formatArgs, pTxtFormat);
            vswprintf_s(buffer, pTxtFormat, formatArgs);
            va_end(formatArgs);

            wprintf_s(buffer);
        }
    };

#ifdef LOG_VERBOSE
#define LogVerbose(S, ...) Botato::Logger::DbgPrintf(L##S##"\n", __VA_ARGS__)
#else
#define LogVerbose(S, ...)
#endif

#ifdef LOG_FUNC_TRANSITION
#define LogEnter() Botato::Logger::DbgPrintf(L##"->" __FUNCTION__ "\n")
#define LogExit() Botato::Logger::DbgPrintf(L##"<-" __FUNCTION__ "\n")
#else
#define LogEnter()
#define LogExit()
#endif

#define LogInfo(S, ...) Botato::Logger::DbgPrintf(L##S##"\n", __VA_ARGS__)
#define LogError(S, ...) Botato::Logger::DbgPrintf(L##"Error: "##S##"\n", __VA_ARGS__)
#define LogWarning(S, ...) Botato::Logger::DbgPrintf(L##"Warning: "##S##"\n", __VA_ARGS__)

    static int MapRange(int n, int n0, int n1, int m0, int m1)
    {
        double p = (double)(n - n0) / (double)(n1 - n0);
        return m0 + (int)((m1 - m0) * p);
    }
} // namespace Botato