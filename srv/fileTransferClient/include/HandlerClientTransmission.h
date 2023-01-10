//
// Created by ylh on 2021/6/3.
//

#ifndef CENTOS6_9_MINI_CLIENT_TRANSMISSION_HANLDER_H
#define CENTOS6_9_MINI_CLIENT_TRANSMISSION_HANLDER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Poller.h"
#include "TcpSocketHandler.h"
#include "HandlerClient.h"
#include "PbMessage.pb.h"

/*
 * @ brief 数据传输连接类 主要处理与服务端的数据传输
 * */
class CHandlerClientTransmission : public CHandlerTcpSocket
{
public:
    /*
    * @ brief 等待传输的文件信息
     * */
    struct SFileInfo
    {
        int m_fd;                // 文件fd
        int m_offset;            // 文件偏移量
        std::string m_file_name; // 文件名
        SFileInfo()
        {
        }

        SFileInfo(const std::string &file_name, int fd = -1, int offset = -1) : m_fd(fd),
                                                                                m_offset(offset),
                                                                                m_file_name(file_name)
        {
        }
    };

private:
    typedef int (CHandlerClientTransmission::*Func)(); // 规定起始函数格式

    long int m_time_start; // 传输开始时间
    long int m_time_end;   // 传输结束时间

protected:
    SFileInfo m_file_info;                           // 传输的文件信息
    const unsigned int MAX_TRANS_SIZE = MAX_MSG_LEN; // 最大单次传输大小
    Func m_start_func;                               // 数据链路客户端 首先要执行的函数
public:
    CHandlerClient *m_control_handler; // 控制链路信息

    //数据链路是依赖于控制链路的 所以数据链路的构造要传入控制链路信息
    CHandlerClientTransmission(CHandlerProxyBase *HandlerProxy, CHandlerClient *control_handler);

    virtual ~CHandlerClientTransmission();

    /*
     * @ brief 实现 处理可读事件 接口
     * */
    int process_input() override;

    /*
     * @ brief 实现 处理可写事件 接口
     * */
    int process_output() override;

    /*
     * @ brief 连接初始化
     * @ param name 连接名
     * @ param ip 服务器ip
     * @ param port 服务器端口
     * @ param func 传输连接要调用的传输函数
     * */
    void init(const std::string &name, const char *ip, uint16_t port, Func func);

    void reset_file_info();

    SFileInfo get_file_info();

    SFileInfo *mutable_file_info();

public:
    /*
     * 以下开始是能够是start_func的函数
     * */
    int transfer_file();

};

#endif //CENTOS6_9_MINI_CLIENT_TRANSMISSION_HANLDER_H
