//
// Created by ylh on 2021/6/2.
//

#ifndef CENTOS6_9_MINI_CTCPSOCKETHANDLER_H
#define CENTOS6_9_MINI_CTCPSOCKETHANDLER_H

#include <list>
#include "TcpHandlerBase.h"
#include "PacketDecoder.h"
#include "PbMessage.pb.h"
#include "ServerBase.h"
#include "Cache.h"

class CServerBase;

class CHandlerTcpSocket : public CHandlerTcpBase
{
protected:
    bool m_need_close;                   // 标识是否需要关闭连接
    int m_r_skip_idx;                    // 接收缓冲区需要跳过的数据下标
    int m_w_skip_idx;                    // 发送缓冲区需要跳过的数据下标
    CCache m_r;                          // 接收缓冲区
    CCache m_w;                          // 发送缓冲区
    std::list<PBMessage> m_wait_message; // 等待发送的消息
    uint16_t m_server_port;              // 服务端 端口
    std::string m_name;                  // 连接 名
    char m_server_addr[128];             // 服务端 ip
    EConnStatus m_status;                // 连接状态
public:
    CPacketDecoder *m_decoder;        // 解码数据包
    CPacketEncoder *m_encoder;        // 编码数据包
    CServerBase *m_transfer_handler;  // 数据传输连接
    std::string m_transfer_file_name; // 要传输的文件名

public:
    CHandlerTcpSocket();

    virtual ~CHandlerTcpSocket();

    /*
     * @ brief 连接初始化
     * @ param name 连接名
     * @ param ip 服务器ip
     * @ param port 服务器端口
     * */
    virtual void init(const std::string &name, const char *ip, uint16_t port);

    /*
     * @ brief 连接服务端
     * @ return 是否连接成功
     * */
    virtual bool connect();

    /*
     * @ brief 重置连接信息
     * */
    void reset();

    /*
     * @ brief 发送数据
     * @ param buff 等待发送的数据
     * @ param len 等待发送的数据长度
     * @ return 实际发送的数据长度 出错的话 返回 -1
     * */
    int send(const char *buff, int len);

    /*
     * @ brief 实现 处理可读事件 接口
     * */
    virtual int process_input();

    /*
     * @ brief 实现 处理可写事件 接口
     * */
    virtual int process_output();

    /*
     * @ brief 追加数据到发送缓冲区等待发送
     * @ param src 需要追加的数据
     * @ param len 追加的数据长度
     * */
    void append_write(const char *src, int len);

    /*
     * @brief 填充等待发送的消息
     * */
    virtual void push_wait_message(PBMessage &message);

    /*
     * @brief 获取名字
     * */
    virtual const std::string get_name()
    {
        return m_name;
    }
    /*
     * @brief 设置是否需要关闭连接
     * */
    virtual void set_need_close(bool sig)
    {
        m_need_close = sig;
    }

    virtual bool get_need_close()
    {
        return m_need_close;
    }


protected:
    /*
     * @ brief 可读事件接口中调用的处理函数
     * */
    virtual EConnStatus handle_input();

    /*
     * @ brief 可写事件接口中调用的处理函数
     * */
    virtual int handle_output();

    /*
     * @ brief 处理添加到poller中时要做的事情
     * @ return 是否能够添加到poller中
     * */
    virtual int on_attach_poller();

    /*
     * @ brief 当接收到完整数据包之后的处理函数
     * @ return 处理结果
     * */
    virtual int on_packet_complete(const char *data, int len);

    /*
     * @ brief 发送协议消息
     * @ return 是否发送成功 -1 表示发送失败 0表示成功
     * */
    virtual int send_msg(PBMessage &message);

    /*
     * @ brief 打印消息内容
     * */
    virtual void print_message(PBMessage &message);

    /*
     * @ brief 是否需要处理 等待处理的消息
     * */
    virtual bool need_process_wait_message();

    /*
     * @ brief 处理等待发送的消息 处理成功 返回0 失败 返回-1
     * */
    virtual int process_wait_message();

    /*
     * @ brief 解析收到的数据
     * @ return 返回数据是否解析成功
     * */
    virtual bool analysis_data(CCache& data);
};

#endif //CENTOS6_9_MINI_CTCPSOCKETHANDLER_H
