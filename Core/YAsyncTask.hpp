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

#ifndef CGPPY_YASYNCTASK_HPP
#define CGPPY_YASYNCTASK_HPP


#include <iostream>
#include <future>
#include <functional>
#include <utility>
#include <type_traits>


template<typename ReturnType, typename... Args>
class YAsyncTask {
public:
    template<typename Func>
    void start(Func&& func, Args&&... args) {
        future = std::async(std::launch::async, std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template<typename ObjType, typename Method>
    void start(ObjType&& obj, Method method, Args&&... args) {
        future = std::async(std::launch::async, [obj = std::forward<ObjType>(obj), method, args...]() -> ReturnType {
            return (obj->*method)(std::forward<Args>(args)...);
        });
    }

    template<typename ObjType, typename Method>
    void start(std::unique_ptr<ObjType>& obj, Method method, Args&&... args) {
        ObjType* rawPtr = obj.get();
        future = std::async(std::launch::async, [rawPtr, method, args...]() -> ReturnType {
            return (rawPtr->*method)(std::forward<Args>(args)...);
        });
    }

    ReturnType getResult() {
        return future.get();
    }

private:
    std::future<ReturnType> future;
};



#endif //CGPPY_YASYNCTASK_HPP
