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

#include "YGLSLFileIncluder.hpp"
#include "YGlobalInterface.hpp"


shaderc_include_result* YGLSLFileIncluder::GetInclude(const char* requested_source,
                                                      shaderc_include_type type,
                                                      const char* requesting_source,
                                                      size_t include_depth) {
    std::string file_path = std::string(PROJECT_SOURCE_DIR) + "/Assets/Shader/GLSL/Common/" + std::string(requested_source);
    std::ifstream file(file_path);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    shaderc_include_result* result = new shaderc_include_result;
    result->source_name = strdup(file_path.c_str());
    result->source_name_length = file_path.length();
    result->content = strdup(content.c_str());
    result->content_length = content.length();

    return result;
}

void YGLSLFileIncluder::ReleaseInclude(shaderc_include_result* include_result) {
    if (include_result) {
        free((void *) include_result->source_name);
        free((void *) include_result->content);
        delete include_result;
    }
}

