//
// Created by ylh on 2021/6/3.
//
#include <google/protobuf/text_format.h>
#include "ServerHandlerProxy.h"

CPacketDecoder CServerHandlerProxy::stat_decoder;
CPacketEncoder CServerHandlerProxy::stat_encoder;

int CServerHandlerProxy::on_attach_poller(CHandlerTcpSocket *handler)
{
    if(nullptr == handler->m_decoder)
    {
        handler->m_decoder = &stat_decoder;
    }
    if(nullptr == handler->m_encoder)
    {
        handler->m_encoder = &stat_encoder;
    }
    handler->enable_input();
    return 0;
}
int CServerHandlerProxy::on_packet_complete(CHandlerTcpSocket* handler, const char* data, int len)
{
    CPBInputPacker packer;
    if (!packer.copy(data, len))
    {
        ErrMsg("packer.copy(data, len) error \n");
        return -1;
    }
    PBMessage msg;
    if(!packer.decode_proto_msg(msg))
    {
        ErrMsg("packer.decode_proto_msg(message) error \n");
        return -1;
    }
    return process_packet(handler,msg);
}

int CServerHandlerProxy::process_packet(CHandlerTcpSocket* handler,const PBMessage& msg)
{
    CProcessManager::instance()->process(handler,msg);
    handler->disable_input();
    handler->enable_output();
    return 0;
}
void CServerHandlerProxy::printfMessage(const PBMessage &message)
{
    std::string strProto;
    google::protobuf::TextFormat::PrintToString(message, &strProto);
    LogMsg("message id[%d]\n",message.msg_case());
    LogMsg("message [%s]\n", strProto.c_str());
}





