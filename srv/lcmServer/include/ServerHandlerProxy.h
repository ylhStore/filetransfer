//
// Created by ylh on 2021/6/3.
//

#ifndef CENTOS6_9_MINI_SERVER_HANDLER_PROXY_H
#define CENTOS6_9_MINI_SERVER_HANDLER_PROXY_H

#include "HandlerProxy.h"
#include "Singleton.h"
#include "PacketDecoder.h"
#include "ProcessorManager.h"
#include "PbMessage.pb.h"

/*
 * @ brief 连接代理类 代替连接处理完整的数据包
 * */
class CServerHandlerProxy : public CHandlerProxyBase, public CSingleton<CServerHandlerProxy>
{
public:
    static CPacketDecoder stat_decoder; // 解码数据包
    static CPacketEncoder stat_encoder; // 编码数据包
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
    virtual int on_packet_complete(CHandlerTcpSocket *handler, const char *data, int len);

    /*
     * @ brief 打印pb数据内容 
     * @ param message pb数据
     * */
    void printfMessage(const PBMessage &message);

    /*
     * @ brief 处理数据包 
     * @ param handler 代替的连接
     * @ param msg 消息数据
     * @ return 返回处理结果
     * */
    int process_packet(CHandlerTcpSocket *handler, const PBMessage &msg);
};

#endif //CENTOS6_9_MINI_SERVER_HANDLER_PROXY_H
