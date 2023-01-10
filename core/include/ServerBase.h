//
// Created by ylh on 2021/6/2.
//

#ifndef CENTOS6_9_MINI_SERVER_ADAPTER_H
#define CENTOS6_9_MINI_SERVER_ADAPTER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Poller.h"
#include "HandlerProxy.h"
#include "TcpSocketHandler.h"

class CHandlerProxyBase;
class CHandlerTcpSocket;

/*
 * @ brief 服务管理基类 监听端口，注册连接
 * */
class CServerBase : public CPollerObj
{
protected:
    int m_accept_cnt;                   // 能够accept的连接数量
    int m_new_fd_cnt;                   // 新连接的套接字数量
    int *m_fd_array;                    // 新连接的套接字数组
    uint16_t m_bind_port;               // 服务绑定的端口
    char m_bind_addr[128];              // 服务绑定的ip
    int m_back_log;                     // 服务的监听队列大小
    sockaddr *m_peer_array;             // 新连接的地址信息数组
    std::string m_name;                 // 服务名
    CHandlerProxyBase *m_handler_proxy; // 连接的代理
public:
    CServerBase(CHandlerProxyBase *HandlerProxy, int accept_cnt, int backlog);

    virtual ~CServerBase();

    /*
     * @ brief 处理链接请求
     * @ param addr 保存新连接的地址信息数组
     * @ param addr_size 地址信息长度
     * @ return 处理结果 -1 表示 没有新的连接请求， 0 表示 能够处理的新连接已经处理完
     * */
    virtual int process_accept(sockaddr *addr, socklen_t *addr_size);

    /*
     * @ brief 为新链接请求 生成连接处理对象 并注册到反应堆中
     * @ param addr 保存新连接的地址信息数组
     * */
    virtual int process_request(sockaddr *addr);

    /*
     * @ brief 处理添加到poller中时要做的事情
     * @ return 是否能够添加到poller中
     * */
    int on_attach_poller() override;

    /*
     * @ brief 实现 处理可读事件 接口
     * */
    int process_input() override;

    /*
     * @ brief 服务初始化
     * @ param name 连接名
     * @ param ip 服务器ip
     * @ param port 服务器端口
     * */
    void init(const std::string &name, const char *ip, uint16_t port);

    /*
     * @ brief 服务绑定到到指定ip和端口
     * @ return 是否绑定成功
     * */
    bool bind();

    /*
     * @ brief 返回服务名
     * @ return 服务名
     * */
    virtual const std::string get_name();

    /*
     * @ brief 提供生成连接的接口
     * @ return 新生成的连接
    */
   virtual CHandlerTcpSocket *CreateHandler();
};

#endif //CENTOS6_9_MINI_SERVER_ADAPTER_H
