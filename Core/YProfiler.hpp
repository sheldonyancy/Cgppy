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

#ifndef CGPPY_YPROFILER_HPP
#define CGPPY_YPROFILER_HPP


#include "YDefines.h"
#include "YAsyncTask.hpp"

#include <iostream>
#include <memory>
#include <mutex>


class YProfiler {
public:
    static YProfiler* instance();

    void run();

    inline u32 renderingFPS() {return this->m_rendering_fps;}
    inline u32 cpuFPS() {return this->m_cpu_fps;}
    inline u32 gpuFPS() {return this->m_gpu_fps;}

    void accumulateRenderingFrameTime(double time);
    void accumulateCpuFrameTime(double time);
    void accumulateGpuFrameTime(double time);

private:
    YProfiler();
    ~YProfiler();

    void functionTimer();

private:
    double m_rendering_frame_time_accumulator = 0;
    u32 m_rendering_frame_count = 0;
    u32 m_rendering_fps = 0;

    double m_cpu_frame_time_accumulator = 0;
    u32 m_cpu_frame_count = 0;
    u32 m_cpu_fps = 0;

    double m_gpu_frame_time_accumulator = 0;
    u32 m_gpu_frame_count = 0;
    u32 m_gpu_fps = 0; 

    std::unique_ptr<YAsyncTask<void>> m_async;

    std::unique_ptr<std::mutex> m_mutex;
};






#endif