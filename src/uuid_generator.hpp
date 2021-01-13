#ifndef __UUID_GENERATOR_HPP__
#define __UUID_GENERATOR_HPP__

#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include <string>
#include <algorithm>

namespace drpc {

class UUIDGenerator {
public:
    UUIDGenerator() {
        uuid_t uuid;
        has_dev_urandom_ = 0;
        int dev_urandom_fd = open("/dev/urandom", O_RDONLY);
        assert(dev_urandom_fd >= 0);

        has_dev_urandom_ = 1;
        close(dev_urandom_fd);

        uuid_generate_random(uuid);
    }

    ~UUIDGenerator() {
        has_dev_urandom_ = 0;
    }

public:
    std::string GenerateRandomUUID() {
        uuid_t uuid;
        GetUUID(uuid);
        return ToString(uuid);
    }

protected:
    std::string ToString(uuid_t uuid) {
        char data[128];
        memset(data, 0, sizeof(data));
        uuid_unparse(uuid, data);
        std::string str(data);
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        return str;
    }

    uuid_t* ToUUID(std::string uuid) {
        uuid_t* uu = (uuid_t*)calloc(1, sizeof(*uu));
        if (!uu) {
            return nullptr;
        }

        int res = uuid_parse(uuid.c_str(), *uu);
        if (res) {
            free(uu);
            return nullptr;
        }

        return uu;
    }

private:
    void GetUUID(uuid_t uuid) {
        uuid_generate_random(uuid);
    }

private:
    int has_dev_urandom_;
};

} /* end namespace drpc */

#endif /* __UUID_GENERATOR_HPP__ */
