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

#include "YAssets.h"
#include "YGlobalInterface.hpp"
#include "YShaderManager.hpp"

#include <map>

static std::map<enum YeAssetsImage, std::string> g_image_file_map;
static std::map<enum YeAssetsShader, std::string> g_glsl_file_map;
static std::map<enum YeAssetsShader, std::string> g_spv_file_map;

void yInitAssets() {
    std::string exe_path = YGlobalInterface::instance()->getPathExecutable();
    std::string project_path = PROJECT_SOURCE_DIR;

    g_image_file_map.emplace(Lenna, exe_path + "/Assets/Image/Lenna.png");
    g_image_file_map.emplace(Baboon, exe_path + "/Assets/Image/Baboon.jpeg");
    g_image_file_map.emplace(Barbara, exe_path + "/Assets/Image/Barbara.jpg");
    g_image_file_map.emplace(Cameraman, exe_path + "/Assets/Image/Cameraman.png");
    g_image_file_map.emplace(Goldhill, exe_path + "/Assets/Image/Goldhill.png");

    g_glsl_file_map.emplace(YeAssetsShader::Output_Vert, project_path + "/Assets/Shader/GLSL/output.vert");
    g_glsl_file_map.emplace(YeAssetsShader::Output_Frag, project_path + "/Assets/Shader/GLSL/output.frag");
    g_glsl_file_map.emplace(YeAssetsShader::Rasterization_Vert, project_path + "/Assets/Shader/GLSL/rasterization.vert");
    g_glsl_file_map.emplace(YeAssetsShader::Rasterization_Frag, project_path + "/Assets/Shader/GLSL/rasterization.frag");
    g_glsl_file_map.emplace(YeAssetsShader::Shadow_Map_Vert, project_path + "/Assets/Shader/GLSL/shadow_map.vert");
    g_glsl_file_map.emplace(YeAssetsShader::Shadow_Map_Frag, project_path + "/Assets/Shader/GLSL/shadow_map.frag");
    g_glsl_file_map.emplace(YeAssetsShader::Path_Tracing_Vert, project_path + "/Assets/Shader/GLSL/path_tracing.vert");
    g_glsl_file_map.emplace(YeAssetsShader::Path_Tracing_Frag, project_path + "/Assets/Shader/GLSL/path_tracing.frag");

    g_spv_file_map.emplace(YeAssetsShader::Output_Vert, exe_path + "/Assets/Shader/spv_glsl/output.vert.spv");
    g_spv_file_map.emplace(YeAssetsShader::Output_Frag, exe_path + "/Assets/Shader/spv_glsl/output.frag.spv");
    g_spv_file_map.emplace(YeAssetsShader::Rasterization_Vert, exe_path + "/Assets/Shader/spv_glsl/rasterization.vert.spv");
    g_spv_file_map.emplace(YeAssetsShader::Rasterization_Frag, exe_path + "/Assets/Shader/spv_glsl/rasterization.frag.spv");
    g_spv_file_map.emplace(YeAssetsShader::Shadow_Map_Vert, exe_path + "/Assets/Shader/spv_glsl/shadow_map.vert.spv");
    g_spv_file_map.emplace(YeAssetsShader::Shadow_Map_Frag, exe_path + "/Assets/Shader/spv_glsl/shadow_map.frag.spv");
    g_spv_file_map.emplace(YeAssetsShader::Path_Tracing_Vert, exe_path + "/Assets/Shader/spv_glsl/path_tracing.vert.spv");
    g_spv_file_map.emplace(YeAssetsShader::Path_Tracing_Frag, exe_path + "/Assets/Shader/spv_glsl/path_tracing.frag.spv");

    YShaderManager::instance();
}

const char* yAssetsImageFile(enum YeAssetsImage e) {
    return g_image_file_map.find(e)->second.c_str();
}

const char* yAssetsGlslFile(enum YeAssetsShader s) {
    return g_glsl_file_map.find(s)->second.c_str();
}

const char* yAssetsSpvFile(enum YeAssetsShader s) {
    return g_spv_file_map.find(s)->second.c_str();
}

u32* getSpvCode(enum YeAssetsShader s) {
    return YShaderManager::instance()->getSpvCode(s);
}

u64 getSpvCodeSize(enum YeAssetsShader s) {
    return YShaderManager::instance()->getSpvCodeSize(s);
}
