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

#include "YGlobalInterface.hpp"
#include "YLogger.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <thread>

YGlobalInterface* YGlobalInterface::instance() {
    static YGlobalInterface global_interface;

    return &global_interface;
}

YGlobalInterface::YGlobalInterface() {

}

YGlobalInterface::~YGlobalInterface() {

}

void YGlobalInterface::init(std::string exe_path_str) {
    this->m_path_executable = exe_path_str;
}

std::string YGlobalInterface::readTextFile(const std::string& file_path) {
    std::ifstream my_file(file_path);
    if (my_file.is_open()) {
        std::stringstream buffer;
        buffer << my_file.rdbuf();
        std::string contents = buffer.str();
        my_file.close();
        return contents;
    } else {
        YERROR("Unable to open file: %s", file_path.c_str());
    }

    return "";
}

std::string YGlobalInterface::getCurrentThreadId() {
    auto thread_id = std::this_thread::get_id();
    std::ostringstream oss;
    oss << thread_id;
    return oss.str();
}

