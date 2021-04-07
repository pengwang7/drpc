#include "logger.h"

int main() {
    drpc::Logger::Instance().Init();

    std::string a = "1";
    int b = 2;
    uint32_t c = 3;
    uint64_t d = 4;

    DTRACE("test trace {}", a);
    DDEBUG("test debug {}", b);
    DINFO("test info {}", c);
    DERROR("test error {}", d);

    drpc::Logger::Instance().Destroy();

    return 0;
}
