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

#ifndef CGPPY_YSHADERMANAGER_HPP
#define CGPPY_YSHADERMANAGER_HPP


#include "YAssets.h"

#include <shaderc/shaderc.hpp>

#include <map>


class YShaderManager{
public:
    static YShaderManager* instance();

    uint32_t* getSpvCode(YeAssetsShader s);
    unsigned int getSpvCodeSize(YeAssetsShader s);

private:
    YShaderManager();
    ~YShaderManager();

private:
    void init();

    bool compileGlslToSpv(YeAssetsShader s, shaderc_shader_kind kind);
    bool readSpv(YeAssetsShader s);

private:
    std::map<YeAssetsShader, std::vector<uint32_t>> m_spv_code_map;
};


#endif //CGPPY_SPIRVMANAGER_HPP
