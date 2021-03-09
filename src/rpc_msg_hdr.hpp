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

#ifndef __MESSAGE_HEADER_HPP__
#define __MESSAGE_HEADER_HPP__

namespace drpc {

enum MessageContentType {
    MSG_CONTENT_TYPE_JSON = 0,

    MSG_CONTENT_TYPE_PROTOBUF = 1,
};

#pragma pack(1)
struct rpc_msg_hdr {
    uint8_t version:2;

    uint8_t type:2;

    uint8_t padding:4;

    uint32_t length;
};
#pragma pack()

typedef struct rpc_msg_hdr rpc_msg_hdr;

} /* end namespace drpc */

#endif /* __MESSAGE_HEADER_HPP__ */
