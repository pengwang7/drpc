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

#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <string>

#include <errno.h>
#include <string.h>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace drpc {

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define DTRACE(...) \
         Logger::Write(0, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__);

#define DDEBUG(...) \
         Logger::Write(1, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__);

#define DINFO(...) \
         Logger::Write(2, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__);

#define DWARNING(...) \
         Logger::Write(3, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__);

#define DERROR(...) \
         Logger::Write(4, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__);

#define DASSERT(cond, ...) \
         if (!(cond)) { \
            DERROR(__VA_ARGS__); \
            _exit(0); \
         }

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void Init(std::string name = "drpc", std::string filename = "log", int level = 0/* = spdlog::level::trace*/) {
        spdlog::flush_every(std::chrono::seconds(2));
        spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
        logger_ = spdlog::rotating_logger_mt(name, filename, 1024 * 1024 * 3, 3);
        logger_->flush_on(spdlog::level::err);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%l]: %v");
    }

    void Destroy() {
        spdlog::shutdown();
    }

    void SetLevel(int level) {
        spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
    }

    std::shared_ptr<spdlog::logger> GetLogger() {
        return logger_;
    }

    static void Write(int level, const char* filename, const char* funcname, std::size_t lineno, const char* format, ...) {
        char details[128] = {0};
        char message[2048] = {0};

        va_list args;
        va_start(args, format);
        vsnprintf(message, sizeof(message), format, args);
        va_end(args);

        snprintf(details, sizeof(details), "(%s:%s:%ld)", filename, funcname, lineno);

        switch (level) {
        case 0:
            Logger::Instance().GetLogger()->trace("{0} {1}", details, message);
            break;
        case 1:
            Logger::Instance().GetLogger()->debug("{0} {1}", details, message);
            break;
        case 2:
            Logger::Instance().GetLogger()->info("{0} {1}", details, message);
            break;
        case 3:
            Logger::Instance().GetLogger()->warn("{0} {1}", details, message);
            break;
        case 4:
            Logger::Instance().GetLogger()->error("{0} {1}", details, message);
            break;
        default:
            break;
        }
    }

private:
    Logger() {}

    ~Logger() {}

    Logger(const Logger&) = delete;

    Logger& operator=(const Logger&) = delete;

private:
    std::shared_ptr<spdlog::logger> logger_;
};

} /* end namespace drpc */

#endif /* __LOGGER_HPP__ */
