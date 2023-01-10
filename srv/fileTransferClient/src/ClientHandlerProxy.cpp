//
// Created by ylh on 2021/6/3.
//

#include "ClientHandlerProxy.h"
#include "PacketDecoder.h"

CPacketDecoder CClientHandlerProxy::stat_decoder;
CPacketEncoder CClientHandlerProxy::stat_encoder;

int CClientHandlerProxy::on_attach_poller(CHandlerTcpSocket *handler)
{
    if (nullptr == handler->m_decoder)
    {
        handler->m_decoder = &stat_decoder;
    }
    if (nullptr == handler->m_encoder)
    {
        handler->m_encoder = &stat_encoder;
    }
    handler->enable_input();
    return 0;
}

int CClientHandlerProxy::on_packet_complete(CHandlerTcpSocket *handler, const char *data, int len)
{
    CPBInputPacker packer;
    if (!packer.copy(data, len))
    {
        ErrMsg("packer.copy(data, len) error \n");
        return -1;
    }
    PBMessage msg;
    if (!packer.decode_proto_msg(msg))
    {
        ErrMsg("packer.decode_proto_msg(message) error \n");
        return -1;
    }
    return process_packet(handler, msg);
}

int CClientHandlerProxy::process_packet(CHandlerTcpSocket *handler, const PBMessage &msg)
{
    CProcessManager::instance()->process(handler, msg);
    handler->disable_output();
    handler->enable_output();
    return 0;
}

