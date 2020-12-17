/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef __BYTE_BUFFER_HPP__
#define __BYTE_BUFFER_HPP__

#include <arpa/inet.h>

#include "buffer.hpp"

namespace drpc {

class ByteBufferWriter {
public:
    explicit ByteBufferWriter(Buffer& buf) : buffer_(buf) {}

    ByteBufferWriter(const ByteBufferReader&) = delete;

    ByteBufferWriter& operator=(const ByteBufferWriter&) = delete;

    ~ByteBufferWriter() {}

    void WriteUInt32(uint32_t val) {
        uint32_t net_x = htonl(val);
        WriteBytes(reinterpret_cast<const char*>(&net_x), sizeof(net_x));
    }

    void WriteUInt64(uint64_t val) {
        int64_t net_x = htonll(val);
        WriteBytes(reinterpret_cast<const char*>(&net_x), sizeof(net_x));
    }

    void WriteBytes(const char* val, size_t len) {
        if (!val || !len) {
            return;
        }

        buffer_.AppendData(val, len);
    }

    void WriteString(const std::string& val) {
        WriteBytes(val.c_str(), val.size());
    }

private:
    Buffer& buffer_;
};

class ByteBufferReader {
public:
    explicit ByteBufferReader(Buffer& buf) : buffer_(buf) {}

    ByteBufferReader(const ByteBufferReader&) = delete;

    ByteBufferReader& operator=(const ByteBufferReader&) = delete;

    ~ByteBufferReader() {}

    bool ReadUInt32(uint32_t* val) {
        if (!val || buffer_.UnreadByteSize() < sizeof(*val)) {
            return false;
        }

        memcpy(&val, buffer_.begin(), sizeof(*val));

        *val = ntohl(*val);

        return true;
    }

    bool ReadUInt64(uint64_t* val) {
        if (!val || buffer_.UnreadByteSize() < sizeof(*val)) {
            return false;
        }

        memcpy(&val, buffer_.begin(), sizeof(*val));

        *val = ntohll(*val);

        return true;
    }

    bool ReadBytes(char* val, size_t len) {
        if (!val || len > buffer_.UnreadByteSize()) {
            return false;
        }

        memcpy(val, buffer_.begin(), len);

        return true;
    }

    bool ReadString(std::string* val, size_t len) {
        if (!val || len > buffer_.UnreadByteSize()) {
            return false;
        }

        val->append(buffer_.begin(), len);

        return true;
    }

    bool Consume(size_t len) {
        if (len > buffer_.UnreadByteSize()) {
            return false;
        }

        buffer_.read_index_ += len;

        return true;
    }

private:
    Buffer& buffer_;
};

} /* end namespace drpc */

#endif /* __BYTE_BUFFER_HPP__ */
