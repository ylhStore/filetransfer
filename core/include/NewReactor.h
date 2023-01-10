//
// Created by ylh on 2021/6/2.
//

#ifndef CENTOS6_9_MINI_NEW_REACTOR_H
#define CENTOS6_9_MINI_NEW_REACTOR_H

#include <map>
#include <signal.h>
#include "Singleton.h"
#include "ServerBase.h"
#include "Poller.h"

#define NEW_REACTOR_MAX_POLLER 102400

// 信号flag
#define FLAG_RUN (0)

/*
 * @ brief 反应堆类 管理CPollerUnit中增删事件以及触发事件调用
 * */
class CNewReactor : public CSingleton<CNewReactor>
{
private:
    typedef std::map<std::string, CServerBase *> ServerMap;
    int m_max_poller;           // 最大 poller 数量
    CPollerManager *m_poller_unit; // poller管理实例
protected:
    ServerMap m_servers; // 监听的所有服务
public:
    /*
     * @ brief 初始化反应堆
     * @ param max_poller 最大的poller 数量
     * @ return 初始化是否成功
     * */
    bool init(int max_poller = NEW_REACTOR_MAX_POLLER);

    /*
     * @ brief 检测监听的信号信息，控制是否程序退出
     * @ return 程序是否需要退出
     * */
    bool check_sig_flag();

    /*
     * @ brief 服务 注册到 poller管理实例
     * @ param adapter 要注册的服务信息
     * @ return 是否注册成功
     * */
    bool bind_server(CServerBase *adapter);

    /*
     * @ brief 连接 注册到 poller管理实例
     * @ param Poller 要注册的连接信息
     * @ return 是否注册成功
     * */
    int register_client(CPollerObj *Poller);

    /*
     * @ brief poller 注册到 poller管理实例
     * @ param Poller 要注册的poller信息
     * @ return 是否注册成功
     * */
    int attach_poller(CPollerObj *Poller);

    /*
     * @ brief 反应堆的循环运行接口
     * @ return 运行结果
     * */
    int run_event_loop();

    /*
     * @ brief 检测是否有某一服务
     * @ param name 服务名
     * @ return 是否有某一服务
     * */
    bool exist_adapter(std::string &name);

    /*
     * @ brief 删除某一服务
     * @ param name 服务名
     * */
    void delete_adapter(std::string &name);
};

#endif //CENTOS6_9_MINI_NEW_REACTOR_H
