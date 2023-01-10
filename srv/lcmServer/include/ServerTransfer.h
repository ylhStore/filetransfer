//
// Created by ylh on 2021/6/11.
//

#ifndef CENTOS6_9_MINI_SERVERTRANSFER_H
#define CENTOS6_9_MINI_SERVERTRANSFER_H

#include "ServerBase.h"
#include "TcpSocketHandler.h"

/*
 * @ brief 数据传输服务管理类 监听端口，注册数据传输连接
 * */
class CServerTransfer : public CServerBase
{
public:
    CHandlerTcpSocket *m_control_handler; // 数据传输连接的控制连接
    std::string m_transfer_file_name;     // 数据传输连接要传输的文件名

public:
    CServerTransfer(CHandlerProxyBase *HandlerProxy, int accept_cnt, int backlog) : CServerBase(HandlerProxy, accept_cnt, backlog),
                                                                                    m_control_handler(nullptr)
    {
    }

    virtual ~CServerTransfer();
    /*
     * @ brief 为新链接请求 生成连接处理对象 并注册到反应堆中
     * @ param addr 保存新连接的地址信息数组
     * */
    int process_request(sockaddr *addr) override;
};

#endif //CENTOS6_9_MINI_SERVERTRANSFER_H
