//
// Created by ylh on 2021/6/8.
//

#ifndef CENTOS6_9_MINI_PROCESSORBASE_H
#define CENTOS6_9_MINI_PROCESSORBASE_H

#include "TcpSocketHandler.h"
#include "PbMessage.pb.h"

/*
 * @ brief 消息处理器基类 定义消息处理接口
 * */
class CProcessorBase{
public:
    /*
     * @ brief 消息处理接口
     * @ param handler 等待处理消息的连接
     * @ param message 等待处理的消息
     * */
    virtual int process(CHandlerTcpSocket* handler, const PBMessage& message) = 0;
};

#endif //CENTOS6_9_MINI_PROCESSORBASE_H
