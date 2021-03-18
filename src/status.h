/*
 * MIT License
 *
 * Copyright (c) 2020 pengwang7
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

#ifndef __STATUS_H__
#define __STATUS_H__

#include <atomic>

namespace drpc {

enum Status {
    kNull = 0,
    kInitializing = 1,
    kInitialized = 2,
    kStarting = 3,
    kRunning = 4,
    kStopping = 5,
    kStopped = 6,
};

class StatusObserver {
public:
    bool IsRunning() const {
        return status_.load() == kRunning;
    }

    bool IsStopping() const {
        return status_.load() == kStopping;
    }

    bool IsStopped() const {
        return status_.load() == kStopped;
    }

    std::string ToString() {
//        switch (status_.load()) {
//        case kNull:
//            return std::string("Null");
//        case kInitializing:
//            return std::string("Initializing");
//        case kStarting:
//            return std::string("Starting");
//        case kRunning:
//            return std::string("Running");
//        case kStopping:
//            return std::string("Stopping");
//        case kStopped:
//            return std::string("Stopped");
//        }

        return std::string("Unknow");
    }

protected:
    std::atomic<Status> status_ = { kNull };
};

} // namespace drpc

#endif // __STATUS_H__
