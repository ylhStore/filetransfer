//
// Created by ylh on 2021/6/2.
//

#ifndef CENTOS6_9_MINI_TCP_HANDLER_BASE_H
#define CENTOS6_9_MINI_TCP_HANDLER_BASE_H

#include <arpa/inet.h>
#include "HandlerProxy.h"
#include "Poller.h"

enum EConnStatus
{
    ECONNECT_IDLE,           // 链接闲置
    ECONNECT_DISCONNECT,     // 链接断开
    ECONNECT_DATA_RECEIVING, // 数据接收中
    ECONNECT_RECV_ERROR,     // 数据接收错误
    ECONNECT_RECV_DONE,      // 数据接收完成
    ECONNECT_DATA_SENDING,   // 数据发送中
    ECONNECT_SEND_ERROR,     // 数据发送错误
    ECONNECT_SEND_DONE,      // 数据发送完成

};

class CHandlerProxyBase;

/*
 * @ brief 连接基类 处理连接的基本信息
 * */
class CHandlerTcpBase : public CPollerObj
{
private:
    sockaddr m_sock_addr; // 连接地址信息
    std::string m_ip;     // 连接的ip
    uint16_t m_port;      // 连接的端口
public:
    CHandlerProxyBase *m_handler_proxy;

public:
    CHandlerTcpBase() : m_handler_proxy(nullptr){};

    virtual ~CHandlerTcpBase(){};

    inline sockaddr get_sock_addr()
    {
        return m_sock_addr;
    }

    inline void set_sock_addr(sockaddr *addr)
    {
        memcpy(&m_sock_addr, addr, sizeof(sockaddr));
    }

    inline std::string get_ip()
    {
        return m_ip;
    }

    inline void set_ip(const std::string &ip)
    {
        m_ip = ip;
    }

    inline uint16_t get_port()
    {
        return m_port;
    }

    inline void set_port(uint16_t port)
    {
        m_port = port;
    }

    inline int get_net_fd()
    {
        return m_net_fd;
    }

    inline void set_net_fd(int net_fd)
    {
        m_net_fd = net_fd;
    }
};

#endif //CENTOS6_9_MINI_TCP_HANDLER_BASE_H
