//
// Created by ylh on 2021/6/2.
//

#ifndef CENTOS6_9_MINI_CPOLLEROBJ_H
#define CENTOS6_9_MINI_CPOLLEROBJ_H

#include <sys/epoll.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <iostream>
#include <string.h>
#include "StaticLink.h"

enum EPollerState
{
    POLLER_FAIL = -1,
    POLLER_SUCCESS,
    POLLER_COMPLETE,
    POLLER_CONTINUE,
};

class CPollerManager;

/*
 * @ brief 监听事件基类
 * */
class CPollerObj
{
protected:
    int m_net_fd;                     // 监听的套接字
    int newEvents;                    // 新的事件类型
    int oldEvents;                    // 旧的事件类型
    CPollerManager *m_owner_unit;     // epoll 实例
    SNode<CPollerObj> *m_poller_node; // poller实例自己在epoll实例中的 结构
public:
    friend class CPollerManager;

    CPollerObj(CPollerManager *owner = nullptr, int fd = 0);

    virtual ~CPollerObj();

    /*
     * @ brief 处理添加到poller中时要做的事情
     * @ return 是否能够添加到poller中
     * */
    virtual int on_attach_poller() = 0;

    /*
     * @ brief 实现 处理可读事件 接口
     * */
    virtual int process_input();

    /*
     * @ brief 实现 处理可写事件 接口
     * */
    virtual int process_output();

    /*
     * @ brief 实现 异常处理 接口
     * */
    virtual int process_hang_up();

    /*
     * @ brief 事件类型添加 EPOLLIN
     * */
    void enable_input()
    {
        newEvents |= EPOLLIN;
    }

    /*
     * @ brief 事件类型删除 EPOLLIN
     * */
    void disable_input()
    {
        newEvents &= ~EPOLLIN;
    }

    /*
     * @ brief 事件类型添加或者删除 EPOLLIN, state为true 则添加，state为false 则删除
     * */
    void modify_input(bool state)
    {
        if (state)
        {
            newEvents |= EPOLLIN;
        }
        else
        {
            newEvents &= ~EPOLLIN;
        }
    }

    /*
     * @ brief 事件类型添加 EPOLLOUT
     * */
    void enable_output()
    {
        newEvents |= EPOLLOUT;
    }

    /*
     * @ brief 事件类型删除 EPOLLOUT
     * */
    void disable_output()
    {
        newEvents &= ~EPOLLOUT;
    }

    /*
     * @ brief 事件类型添加或者删除 EPOLLOUT, state为true 则添加，state为false 则删除
     * */
    void modify_output(bool state)
    {
        if (state)
        {
            newEvents |= EPOLLOUT;
        }
        else
        {
            newEvents &= ~EPOLLOUT;
        }
    }

    /*
     * @ brief 添加到 epoll监听树中
     * */
    int attach_poller(CPollerManager *unit);

    /*
     * @ brief 从epoll监听树中删除
     * */
    int detach_poller();

    /*
     * @ brief 添加事件
     * */
    int apply_events();
};

/*
 * @ brief 管理实例类 管理poller
 * */
class CPollerManager
{
private:
    int m_epoll_fd;                    // epoll create创建的文件描述符
    int m_max_pollers;                 // 最大 poller 数量
    int m_used_pollers;                // 正在使用的 poller数量
    int m_wait_nums;                   // epoll 中就绪的事件数量
    epoll_event *m_ep_events;          // epoll事件数组
    CStaticLink<CPollerObj> m_pollers; // poller链表

public:
    friend class CPollerObj;

    CPollerManager(int mp);

    ~CPollerManager();

    /*
     * @ brief 设置最大poller数量
     * */
    int set_max_pollers(int mp);

    /*
     * @ brief 获得最大poller数量
     * */
    int get_max_pollers() const
    {
        return m_max_pollers;
    }

    /*
     * @ brief 初始化管理实例
     * */
    int init_poller_unit();

    /*
     * @ brief 调用 epoll_wait 等待事件
     * @ param timeout 最大等待事件 单位毫秒
     * @ return epoll_wait调用是否出错
     * */
    bool wait_poller_events(int timeout);

    /*
     * @ brief 处理当前等待的事件
     * */
    void process_poller_events();

    int get_epoll_fd() const
    {
        return m_epoll_fd;
    }

    /*
     * @ brief 获得当前使用的poller数量
     * @ return 当前使用的poller数量
    */
    int get_used_pollers() const
    {
        return m_used_pollers;
    }

    /*
     * @ brief 判断是否是静态链表中的空链表头结点
     * @ return 是否是静态链表中的空链表头结点
    */
    bool is_null_poller(SNode<CPollerObj>* poller)
    {
        return m_pollers.is_null_node(poller);
    }

private:
    /*
     * @ brief 初始化poller数组
     * */
    bool init_m_pollers();
    /*
     * @ brief 释放poller
     * */
    void free_poller(SNode<CPollerObj> *poller_packet);

    /*
     * @ brief 申请poller
     * */
    SNode<CPollerObj> *alloc_poller();
    /*
     * @ brief 封装 epoll_ctl操作
     * @ param op 要进行的操作 , EPOLLIN,EPOLLOUT等
     * @ param fd 要进行操作的文件描述符
     * @ param ev 放到监听树上监听事件
     * */
    int epoll_ctl(int op, int fd, epoll_event *ev);
};

#endif //CENTOS6_9_MINI_CPOLLEROBJ_H
