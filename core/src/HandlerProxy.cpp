//
// Created by ylh on 2021/6/3.
//

#include "HandlerProxy.h"

int CHandlerProxyBase::on_attach_poller(CHandlerTcpSocket *handler)
{
    return 0;
}

int CHandlerProxyBase::on_packet_complete(CHandlerTcpSocket* handler, const char* data, int len)
{
    return 0;
}