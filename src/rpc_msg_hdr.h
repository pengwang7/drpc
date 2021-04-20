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

#ifndef __MESSAGE_HEADER_H__
#define __MESSAGE_HEADER_H__

#include <endian.h>

namespace drpc {

enum MessageContentType {
    MSG_CONTENT_TYPE_JSON = 0,

    MSG_CONTENT_TYPE_PROTOBUF = 1,
};

enum RpcPayloadType {
    PAYLOAD_TYPE_JSON = 0,

    PAYLOAD_TYPE_PROTOBUF = 1,
};

#pragma pack(1)

class RpcPacket {
public:
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

    struct Body {
        std::string content;
    };

public:
    static RpcPacket* CreateRpcPacket(Buffer& buffer, ByteBufferReader* io_reader) {
        Header header;
        memset(&header, 0, sizeof(header));

        if (!io_reader->ReadBytes(reinterpret_cast<char*>(&header), sizeof(header))) {
            DWARNING("The buffer unread bytes < sizeof(header).");
            return nullptr;
        }

        uint32_t body_length = static_cast<uint32_t>(ntohl(header.length));

        //DDEBUG("payload_type: {}, body_length: {}.", header.payload, body_length);

        Body body;

        if (buffer.UnreadByteSize() < body_length + sizeof(header)) {
            DWARNING("Data is not complete.");
            return nullptr;
        }

        io_reader->Consume(sizeof(header));

        if (!io_reader->ReadString(&body.content, body_length)) {
            DERROR("Read data from buffer failed.");
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
    uint8_t GetVersion() {
        return header_.version;
    }

    uint8_t GetPayloadType() {
        return header_.payload;
    }

    uint32_t GetBodyLength() {
        return static_cast<uint32_t>(ntohl(header_.length));
    }

    std::string GetBody() {
        return body_.content;
    }

private:
    Header header_;

    Body body_;
};

struct rpc_msg_hdr {
    uint8_t version:2;

    uint8_t type:2;

    uint8_t padding:4;

    uint32_t length;
};
#pragma pack()

typedef struct rpc_msg_hdr rpc_msg_hdr;

} // namespace drpc

#endif // __MESSAGE_HEADER_H__
