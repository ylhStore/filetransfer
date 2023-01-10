//
// Created by ylh on 2021/6/2.
//

#ifndef CENTOS6_9_MINI_CSERVERTRANSMiSSION_H
#define CENTOS6_9_MINI_CSERVERTRANSMiSSION_H

#include <list>
#include <sys/stat.h>
#include "TcpSocketHandler.h"
#include "ServerTransfer.h"
#include "PbMessage.pb.h"

extern const unsigned int MAX_RECV_LEN;

class CHandlerTransmission : public CHandlerTcpSocket
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
        struct stat m_file_st;   // 文件信息
        SFileInfo()
        {
        }

        SFileInfo(const std::string &file_name, int fd = -1, int offset = -1) : m_fd(fd),
                                                                                m_offset(offset),
                                                                                m_file_name(file_name)
        {
        }
    };

protected:
    uint16_t m_server_port;                                 // 服务端 端口
    std::string m_name;                                     // 连接 名
    char m_server_addr[128];                                // 服务端 ip
    SFileInfo m_file_info;                                  // 传输的文件信息
    const unsigned int MAX_TRANS_SIZE = MAX_TCP_BUFFER_LEN; // 最大单次传输大小

public:
    CServerTransfer *m_control_handler; // 控制链路信息

public:
    CHandlerTransmission(CHandlerProxyBase *HandlerProxy, CServerTransfer *control_handler);

    virtual ~CHandlerTransmission();

    void reset_file_info();

    SFileInfo &get_file_info();

    SFileInfo *mutable_file_info();
};

#endif //CENTOS6_9_MINI_CSERVERTRANSMiSSION_H
