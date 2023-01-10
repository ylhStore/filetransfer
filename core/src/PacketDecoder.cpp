//
// Created by ylh on 2021/6/3.
//
#include "PacketDecoder.h"

CPacket::CPacket()
{}

CPacket::~CPacket()
{}

int CPacket::get_head_len()
{
    return sizeof(SPacketHeader);
}

bool CPacket::check_head(const char *data, int len)
{
    SPacketHeader *header = (SPacketHeader *) data;
    return 't' == header->check_code[0]
           && 'e' == header->check_code[1]
           && 's' == header->check_code[2]
           && 't' == header->check_code[3];
}

int CPacket::get_packet_len(const char *data, int len)
{
    SPacketHeader *header = (SPacketHeader *) data;
    unsigned int pkg_len = ntohl(header->pack_len);
    if(pkg_len < sizeof (SPacketHeader) || pkg_len > MAX_RECV_LEN)
    {
        return -1;
    }else{
        return pkg_len;
    }
}

CPacketDecoder::CPacketDecoder()
{}

CPacketDecoder::~CPacketDecoder()
{}

int CPacketDecoder::unpack(const char *data, int len)
{
    //不足一个包头, 继续接收
    if(len < get_head_len())
    {
        return 0;
    }
    //满足一个包头, 判断包头合法性
    if(!check_head(data,len))
    {
        ErrMsg("包头不合法[%s]\n",data);
        return -1;
    }
    //满足一个包头, 判断整包长合法性
    int pkg_len = get_packet_len(data,len);
    if(pkg_len < 0)
    {
        ErrMsg("整包长不合法\n");
        return -1;
    }
    // 包体未接收完整
    if(len < pkg_len)
    {
        return 0;
    }
    // 收到一个完整的包
    return pkg_len;
}

CPacketEncoder::CPacketEncoder()
{}

CPacketEncoder::~CPacketEncoder()
{}

int CPacketEncoder::pack(const char *data, int len, std::string &dest)
{
    LogMsg("dest[%s]\n",dest.c_str());
    return 0;
}

