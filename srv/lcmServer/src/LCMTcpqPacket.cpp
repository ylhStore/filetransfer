#include"LCMTcpqPacket.h"
int CLCMTcpqPacketDecoder::get_head_len()
{
    return 0;
}

bool CLCMTcpqPacketDecoder::check_head(const char *data, int len)
{
    return true;
}

int CLCMTcpqPacketDecoder::get_packet_len(const char *data, int len)
{
    return 1;
}

int CLCMTcpqPacketDecoder::unpack(const char *data, int len)
{
    return 1;
}