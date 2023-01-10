//
// Created by ylh on 2021/6/3.
//

#ifndef CENTOS6_9_MINI_CLIENT_ADAPTER_H
#define CENTOS6_9_MINI_CLIENT_ADAPTER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Poller.h"
#include "TcpSocketHandler.h"
#include "PbMessage.pb.h"

/*
 * @ brief 控制连接类 主要处理与服务端的命令交互
 * */
class CHandlerClient : public CHandlerTcpSocket
{
protected:
    std::string m_tmp_str;       // 临时存储对话框输入字符串
    std::string m_aim_file_name; // 要传输的文件名

public:
    CHandlerTcpSocket *m_transmission_handler; // 数据传输链路信息

    CHandlerClient(CHandlerProxyBase *HandlerProxy);

    virtual ~CHandlerClient();

    /*
     * @ brief 实现 处理可读事件 接口
     * */
    int process_input() override;

    /*
     * @ brief 实现 处理可写事件 接口
     * */
    int process_output() override;

    /*
     * @ brief 控制台输入数据 
     * @ param cmd 控制台输入的命令
     * @ param body 命令后跟着的内容 例如: 文件名等，不同的命令可以跟不同的内容
     * @ return 返回值目前没用
     * */
    int terminal_input(std::string &cmd, std::string &body);

    /*
     * @ brief 填充 请求消息
     * */
    int fill_request_message(std::string &cmd, std::string &body, PBMessage &message);

    std::string get_aim_file_name();

    void set_aim_file_name(std::string &aim)
    {
        m_aim_file_name = aim;
    }

    void reset_aim_file_name()
    {
        m_aim_file_name = "";
    }

protected:
    /*
     * @ brief 可读事件接口中调用的处理函数
     * */
    int handle_output() override;
};

#endif //CENTOS6_9_MINI_CLIENT_ADAPTER_H
