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
#include "YAssets.h"
#include "YAssetManager.hpp"
#include "YShaderManager.hpp"
#include "YMdlaImporter.hpp"
#include "YRendererFrontendManager.hpp"
#include "YRendererBackendManager.hpp"

#include <boost/dll/runtime_symbol_info.hpp>


int main(int argc, char *argv[]) {
    std::string exe_path = boost::dll::program_location().parent_path().string();
    yLogInit(exe_path.c_str());
    YINFO("Executable Path: %s", exe_path.c_str());

    std::string main_thread_id = YGlobalInterface::instance()->getCurrentThreadId();
    YINFO("Main Thread ID: %s", main_thread_id.c_str());

    YGlobalInterface::instance()->init(exe_path.c_str());

    yInitAssets();

    //
    YRendererFrontendManager::instance()->initFrontend();
    YRendererBackendManager::instance()->initBackend();

    //YAssetManager::instance()->loadAsset<YStlImporter>(yAssetsModelFile(Teapot));
    YAssetManager::instance()->loadAsset<YMdlaImporter>("");

    YRendererFrontendManager::instance()->eventLoop();

    return 0;
}
