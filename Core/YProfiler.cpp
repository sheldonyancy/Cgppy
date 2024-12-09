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


#include "YProfiler.hpp"

#include <chrono>
#include <thread>

YProfiler* YProfiler::instance() {
    static YProfiler profiler;
    return &profiler;
}

YProfiler::YProfiler() {
    this->m_async = std::make_unique<YAsyncTask<void>>();
    this->m_mutex = std::make_unique<std::mutex>();
}

YProfiler::~YProfiler() {

}

void YProfiler::run() {
    this->m_async->start(this, &YProfiler::functionTimer);
}

void YProfiler::functionTimer() {
    while(true) {
        this->m_mutex->lock();
        
        this->m_rendering_fps = 1000.0 / (this->m_rendering_frame_time_accumulator / this->m_rendering_frame_count);
        this->m_rendering_frame_time_accumulator = 0;
        this->m_rendering_frame_count = 0;
          
        this->m_cpu_fps = 1000.0 / (this->m_cpu_frame_time_accumulator / this->m_cpu_frame_count);
        this->m_cpu_frame_time_accumulator = 0;
        this->m_cpu_frame_count = 0;

        this->m_gpu_fps = 1000.0 / (this->m_gpu_frame_time_accumulator / this->m_gpu_frame_count);
        this->m_gpu_frame_time_accumulator = 0;
        this->m_gpu_frame_count = 0;
        
        this->m_mutex->unlock();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void YProfiler::accumulateRenderingFrameTime(double time) {
    this->m_mutex->lock();
    this->m_rendering_frame_time_accumulator += time; 
    this->m_rendering_frame_count++;
    this->m_mutex->unlock();
}

void YProfiler::accumulateCpuFrameTime(double time) {
    this->m_mutex->lock();
    this->m_cpu_frame_time_accumulator += time;
    this->m_cpu_frame_count++; 
    this->m_mutex->unlock();
}

void YProfiler::accumulateGpuFrameTime(double time) {
    this->m_mutex->lock();
    this->m_gpu_frame_time_accumulator += time; 
    this->m_gpu_frame_count++;
    this->m_mutex->unlock();
}
