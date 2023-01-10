//
// Created by ylh on 2021/6/3.
//

#ifndef CENTOS6_9_MINI_HANDLER_PROXY_H
#define CENTOS6_9_MINI_HANDLER_PROXY_H

#include "TcpSocketHandler.h"

class CHandlerTcpSocket;

/*
 * @ brief 连接代理类基类 代替连接处理完整的数据包
 * */
class CHandlerProxyBase
{
public:
    /*
     * @ brief 处理连接添加到poller中时要做的事情
     * @ return 是否能够添加到poller中
     * */
    virtual int on_attach_poller(CHandlerTcpSocket *handler);

    /*
     * @ brief 处理完整的数据包 
     * @ param handler 代替的连接
     * @ param data 数据包内容
     * @ param len 数据包长度
     * @ return 返回处理结果
     * */
    virtual int on_packet_complete(CHandlerTcpSocket* handler, const char* data, int len);
};

#endif //CENTOS6_9_MINI_HANDLER_PROXY_H
