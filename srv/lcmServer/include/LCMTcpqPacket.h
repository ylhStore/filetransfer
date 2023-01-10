#include "PacketDecoder.h"

struct CLCMTcpqHeader
{
};

class CLCMTcpqPacketDecoder : public CPacketDecoder
{
public:
    CLCMTcpqPacketDecoder() {}
    virtual ~CLCMTcpqPacketDecoder() {}

    virtual int get_head_len() override;

    virtual bool check_head(const char *data, int len) override;

    virtual int get_packet_len(const char *data, int len) override;

    virtual int unpack(const char *data, int len) override;
};

class CLCMTcpqInputPacker : public CInputPacker
{
public:
    CLCMTcpqInputPacker():CInputPacker(sizeof(CLCMTcpqHeader)){}
    void end()
    {
    }
};

class CLCMTcpqOutputPacker : public COutputPacket
{
public:
    CLCMTcpqOutputPacker():COutputPacket(sizeof(CLCMTcpqHeader)){}
    void end()
    {
    }
};