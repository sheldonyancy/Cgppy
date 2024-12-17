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

#include "YShaderManager.hpp"
#include "YGlobalInterface.hpp"
#include "YGLSLFileIncluder.hpp"
#include "YLogger.h"

#include <iostream>
#include <filesystem>


YShaderManager* YShaderManager::instance() {
    static YShaderManager* m = new YShaderManager;
    return m;
}

YShaderManager::YShaderManager() {
    this->init();
}

YShaderManager::~YShaderManager() {

}

uint32_t* YShaderManager::getSpvCode(YeAssetsShader s) {
    if (this->m_spv_code_map.contains(s))
    {
        return &this->m_spv_code_map[s][0];
    } else {
        return nullptr;
    }
}

unsigned int YShaderManager::getSpvCodeSize(YeAssetsShader s) {
    if (this->m_spv_code_map.contains(s))
    {
        return this->m_spv_code_map[s].size() * sizeof(uint32_t);
    } else {
        return 0;
    }
}

void YShaderManager::init() {
#ifndef NDEBUG
    this->compileGlslToSpv(YeAssetsShader::Output_Vert, shaderc_vertex_shader);
    this->compileGlslToSpv(YeAssetsShader::Output_Frag, shaderc_fragment_shader);
    this->compileGlslToSpv(YeAssetsShader::Rasterization_Vert, shaderc_vertex_shader);
    this->compileGlslToSpv(YeAssetsShader::Rasterization_Frag, shaderc_fragment_shader);
    this->compileGlslToSpv(YeAssetsShader::Shadow_Map_Vert, shaderc_vertex_shader);
    this->compileGlslToSpv(YeAssetsShader::Shadow_Map_Frag, shaderc_fragment_shader);
    this->compileGlslToSpv(YeAssetsShader::Path_Tracing_Comp, shaderc_compute_shader);
#else
    this->readSpv(YeAssetsShader::Output_Vert);
    this->readSpv(YeAssetsShader::Output_Frag);
    this->readSpv(YeAssetsShader::Rasterization_Vert);
    this->readSpv(YeAssetsShader::Rasterization_Frag);
    this->readSpv(YeAssetsShader::Shadow_Map_Vert);
    this->readSpv(YeAssetsShader::Shadow_Map_Frag);
    this->readSpv(YeAssetsShader::Path_Tracing_Comp);
#endif
}

bool YShaderManager::compileGlslToSpv(YeAssetsShader s, shaderc_shader_kind kind) {
    std::vector<uint32_t> spv_code;
    std::string glsl_file_path = yAssetsGlslFile(s);
    std::string text = YGlobalInterface::instance()->readTextFile(glsl_file_path);

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Customize the compilation options...
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetIncluder(std::make_unique<YGLSLFileIncluder>());
    //options.AddMacroDefinition("GL_EXT_debug_printf");

    // Compile the shader to SPIR-V.
    std::string shader_file_name = std::filesystem::path(glsl_file_path).filename();
    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(text,
                                                                     kind,
                                                                     shader_file_name.c_str(),
                                                                     options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        std::string error_msg = module.GetErrorMessage();
        YERROR("%s compilation failed: %s", glsl_file_path.c_str(), error_msg.c_str());
        return false;
    }

    spv_code = {module.cbegin(), module.cend()};
    this->m_spv_code_map.emplace(s, spv_code);

    std::string spv_file_path = yAssetsSpvFile(s);
    
    std::ofstream out_file(spv_file_path.c_str(), std::ios::binary);
    if (!out_file) {
        YERROR("Cannot open file for writing: %s", spv_file_path.c_str());
    }
    out_file.write(reinterpret_cast<const char*>(&spv_code[0]), spv_code.size() * 4);
    out_file.close();

    return true;
}

bool YShaderManager::readSpv(YeAssetsShader s) {
    std::string spv_file_path = yAssetsSpvFile(s);
    std::ifstream file(spv_file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        YERROR( "Failed to open file: %s", spv_file_path.c_str());
        return false;
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*) buffer.data(), file_size);
    file.close();

    this->m_spv_code_map.emplace(s, buffer);

    return true;
}