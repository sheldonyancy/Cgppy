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

#ifndef CGPPY_YRENDERERBACKENDMANAGER_HPP
#define CGPPY_YRENDERERBACKENDMANAGER_HPP


#include "YRendererBackend.hpp"

struct GLFWwindow;

#include <memory>
#include <map>


class YRendererBackendManager {
public:
    static YRendererBackendManager* instance();

    void initBackend();

    inline YRendererBackend* backend() {return this->m_renderer_backend.find(this->m_current_renderer_backend_api)->second.get();}

    u32 samples();

private:
    YRendererBackendManager();
    ~YRendererBackendManager();

private:
    YeRendererBackendApi m_current_renderer_backend_api;
    std::map<YeRendererBackendApi, std::unique_ptr<YRendererBackend>> m_renderer_backend;
};


#endif //CGPPY_YRENDERERBACKENDMANAGER_HPP
