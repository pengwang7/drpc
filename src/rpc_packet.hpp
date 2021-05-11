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

#ifndef __RPC_PACKET_HPP__
#define __RPC_PACKET_HPP__

#include <endian.h>

#include "byte_buffer.hpp"
#include "constructor_magic.hpp"

namespace drpc {

enum RpcPayloadType {
    PAYLOAD_TYPE_JSON = 0,

    PAYLOAD_TYPE_PROTO = 1,
};

class RpcPacket {
public:
    // Struct for RPC header.
#pragma pack(1)
    struct Header {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
        uint8_t padding:1;
        uint8_t extension:3;
        uint8_t version:4;
        uint8_t payload:7;
        uint8_t marker:1;
#elif (__BYTE_ORDER == __BIG_ENDIAN)
        uint8_t version:4;
        uint8_t padding:1;
        uint8_t extension:3;
        uint8_t marker:1;
        uint8_t payload:7;
#endif
        uint32_t length;
    };
#pragma pack()

    // Struct for RPC body.
    struct Body {
        std::string content;
    };

public:
    static RpcPacket* Parse(Buffer& buffer, ByteBufferReader* io_reader) {
        Header header;
        memset(&header, 0, sizeof(header));

        if (!io_reader->ReadBytes(reinterpret_cast<char*>(&header), sizeof(header))) {
            return nullptr;
        }

        uint32_t body_length = static_cast<uint32_t>(ntohl(header.length));

        Body body;

        if (buffer.UnreadByteSize() < body_length + sizeof(header)) {
            return nullptr;
        }

        io_reader->Consume(sizeof(header));

        if (!io_reader->ReadString(&body.content, body_length)) {
            DERROR("Read failed, packet discarded.");
            io_reader->Consume(body_length);
            return nullptr;
        }

        io_reader->Consume(body_length);

        RpcPacket* packet = new RpcPacket(header, body);

        return packet;
    }

    RpcPacket(Header header, Body body) : header_(header), body_(body) {
        DTRACE("Construct RpcPacket.");
    }

    ~RpcPacket() {
        DTRACE("Destructor RpcPacket.");
    }

public:
    uint8_t GetVersion() const {
        return header_.version;
    }

    uint8_t GetPayloadType() const {
        return header_.payload;
    }

    uint32_t GetBodyLength() const {
        return static_cast<uint32_t>(ntohl(header_.length));
    }

    std::string GetBody() const {
        return body_.content;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(RpcPacket);

    Header header_;

    Body body_;
};

} // namespace drpc

#endif // __RPC_PACKET_HPP__
