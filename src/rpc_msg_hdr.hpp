#ifndef __MESSAGE_HEADER_HPP__
#define __MESSAGE_HEADER_HPP__

namespace drpc {

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
