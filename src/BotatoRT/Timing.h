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

namespace BotatoRT {

    class Delay
    {
    public:

        static void Init()
        {
            (void)QueryPerformanceFrequency(&HpcFreq);

            HpcPerdiodNs = 1000000000.0 / HpcFreq.QuadPart;
            HpcMicroNumTicks = (double)HpcFreq.QuadPart / 1000000.0;
        }

        static void Micro(unsigned us)
        {
            LARGE_INTEGER t0, t1, dTicks;

            dTicks.QuadPart = (LONGLONG)((double)us * HpcMicroNumTicks);

            QueryPerformanceCounter(&t0);

            for (;;) {
                QueryPerformanceCounter(&t1);
                if (t1.QuadPart - t0.QuadPart > dTicks.QuadPart) {
#ifdef DEBUG_HPC
                    LARGE_INTEGER elapsedMicros;

                    elapsedMicros.QuadPart = t1.QuadPart - t0.QuadPart;
                    elapsedMicros.QuadPart *= 1000000;
                    elapsedMicros.QuadPart /= HpcFreq.QuadPart;

                    LogInfo("MicroDelay(%dus) quitted after %lldus", us, elapsedMicros.QuadPart);
#endif
                    return;
                }
            }
        }

        static void Milli(unsigned ms)
        {
            Delay::Micro(ms * 1000);
        }

    private:

        static LARGE_INTEGER HpcFreq;
        static double HpcMicroNumTicks;
        static double HpcPerdiodNs;
    };

    class HpcTimer
    {
    public:

        HpcTimer()
        {
            T0.QuadPart = 0LL;
            T1.QuadPart = 0LL;

            (void)QueryPerformanceFrequency(&TimerFreq);
        }

        void Start()
        {
            (void)QueryPerformanceCounter(&T0);
        }

        void Stop()
        {
            (void)QueryPerformanceCounter(&T1);
        }

        double OperationsPerSecond(int numOps) const
        {
            return (double)numOps / ElapsedSeconds();
        }

        double ElapsedSecondsNow() const
        {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);

            return (double)(now.QuadPart - T0.QuadPart) /
                (double)TimerFreq.QuadPart;
        }

        double ElapsedSeconds() const
        {
            return (double)(T1.QuadPart - T0.QuadPart) /
                (double)TimerFreq.QuadPart;
        }

    private:
        LARGE_INTEGER TimerFreq;
        LARGE_INTEGER T0;
        LARGE_INTEGER T1;
    };
} // namespace BotatoRT 