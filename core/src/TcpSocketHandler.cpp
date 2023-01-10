//
// Created by ylh on 2021/6/2.
//
#include <google/protobuf/text_format.h>
#include "TcpSocketHandler.h"

extern const unsigned int MAX_RECV_LEN;

CHandlerTcpSocket::CHandlerTcpSocket() : m_need_close(false),
                                         m_r_skip_idx(0),
                                         m_w_skip_idx(0),
                                         m_r(MAX_RECV_LEN),
                                         m_w(MAX_RECV_LEN),
                                         m_status(ECONNECT_IDLE),
                                         m_decoder(nullptr),
                                         m_encoder(nullptr)
{
}

CHandlerTcpSocket::~CHandlerTcpSocket()
{
    reset();
}

void CHandlerTcpSocket::init(const std::string &name, const char *ip, uint16_t port)
{
    m_name = name;
    m_server_port = port;
    strncpy(m_server_addr, ip, sizeof(m_server_addr) - 1);
}

bool CHandlerTcpSocket::connect()
{
    m_net_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == m_net_fd)
    {
        ErrMsg("CClientAdapter::connect, socket error[%s]\n", strerror(errno));
        return false;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_server_port);
    addr.sin_addr.s_addr = inet_addr(m_server_addr);

    if (-1 == ::connect(m_net_fd, (sockaddr *)&addr, sizeof(addr)))
    {
        close(m_net_fd);
        ErrMsg("connect error[%s] \n", strerror(errno));
        return false;
    }

    CPollerObj::enable_output();
    return true;
}

void CHandlerTcpSocket::reset()
{
    LogMsg("reset socket handler,fd[%d]\n", m_net_fd);
    m_r.reset();
    m_w.reset();
    disable_input();
    disable_output();
    apply_events();
    detach_poller();
    if (m_net_fd > 0)
    {
        close(m_net_fd);
    }
    m_net_fd = -1;
    m_status = ECONNECT_IDLE;
}

int CHandlerTcpSocket::send(const char *buff, int len)
{
    if (len > 0)
    {
        if (m_w.data_len() == 0)
        {
            int ret = ::send(m_net_fd, buff, len, 0);
            if (-1 == ret)
            {
                if (errno == EINTR || errno == EAGAIN || errno == EINPROGRESS)
                {
                    m_w.append(buff, len);
                    enable_output();
                    apply_events();
                    m_status = ECONNECT_DATA_SENDING;
                    return 0;
                }
                else
                {
                    ErrMsg("error -1 == ret\n");
                    return -1;
                }
            }
            else if (ret < len)
            {
                const char *send_buff = buff;
                send_buff += ret;
                int send_len = len - ret;
                m_w.append(send_buff, send_len);
                enable_output();
                apply_events();
                m_status = ECONNECT_DATA_SENDING;
                return ret;
            }
            else if (ret == len)
            {
                m_status = ECONNECT_SEND_DONE;
                return ret;
            }
        }
        else
        {
            m_w.append(buff, len);
            if (handle_output() == ECONNECT_SEND_ERROR)
            {
                return -1;
            }
            else
            {
                return len;
            }
        }
    }
    return len;
}

int CHandlerTcpSocket::on_attach_poller()
{
    return m_handler_proxy->on_attach_poller(this);
}

int CHandlerTcpSocket::on_packet_complete(const char *data, int len)
{
    return m_handler_proxy->on_packet_complete(this, data, len);
}

int CHandlerTcpSocket::process_input()
{
    LogMsg("process_input\n");
    EConnStatus stage = handle_input();

    switch (stage)
    {
    case ECONNECT_RECV_ERROR:
        delete this;
        return POLLER_COMPLETE;
    case ECONNECT_DISCONNECT:
        delete this;
        return POLLER_COMPLETE;
    case ECONNECT_RECV_DONE:
        return POLLER_SUCCESS;
    case ECONNECT_DATA_RECEIVING:
        return POLLER_CONTINUE;
    default:
        delete this;
        return POLLER_COMPLETE;
    }
}

EConnStatus CHandlerTcpSocket::handle_input()
{
    int curr_recv_len = 0;
    char curr_recv_buf[MAX_RECV_LEN];
    memset(curr_recv_buf, 0, sizeof(curr_recv_buf));
    curr_recv_len = recv(m_net_fd, curr_recv_buf, MAX_RECV_LEN, 0);

    if (-1 == curr_recv_len)
    {
        if (errno != EAGAIN && errno != EINTR && errno != EINPROGRESS)
        {
            disable_input();
            m_status = ECONNECT_RECV_ERROR;
            ErrMsg("recv failed from fd[%d], msg[%s]\n", m_net_fd, strerror(errno));
        }
        else
        {
            m_status = ECONNECT_DATA_RECEIVING;
        }
    }
    else if (0 == curr_recv_len)
    {
        disable_input();
        m_status = ECONNECT_DISCONNECT;
        LogMsg("connection disconnect by user fd[%d],msg[%s]\n", m_net_fd, strerror(errno));
    }
    else
    {
        m_r.append(curr_recv_buf, curr_recv_len);
        while (m_r.data_len() > 0)
        {
            if (!analysis_data(m_r))
            {
                break;
            }
        }
    }

    return m_status;
}

int CHandlerTcpSocket::process_output()
{
    /*
     * 如果处理了等待发送的消息 则本次事件响应结束
     * */
    if (need_process_wait_message())
    {
        if (-1 == process_wait_message())
        {
            delete this;
            return POLLER_COMPLETE;
        }
        else
        {

            // 判断是否需要关闭 链接 , 在消息处理之后 可能要需要关闭连接 但是处理消息的地方 和 发送消息的地方不在一起
            // 所以 在处理消息的地方 只是设置是否关闭 在这里发送完消息之后 才关闭
            if (get_need_close())
            {
                delete this;
                return POLLER_COMPLETE;
            }

            disable_output();
            enable_input();
            return POLLER_SUCCESS;
        }
    }

    /*
     * 没有等待发送的消息处理 则进入默认的处理模式
     * */
    LogMsg("process_output\n");
    int stage = handle_output();

    // 判断是否需要关闭 链接 , 在消息处理之后 可能要需要关闭连接 但是处理消息的地方 和 发送消息的地方不在一起
    // 所以 在处理消息的地方 只是设置是否关闭 在这里发送完消息之后 才关闭
    if (get_need_close())
    {
        delete this;
        return POLLER_COMPLETE;
    }

    switch (stage)
    {
    case ECONNECT_SEND_DONE:
        disable_output();
        enable_input();
        return POLLER_SUCCESS;
    case ECONNECT_DATA_SENDING:
        break;
    case ECONNECT_SEND_ERROR:
        delete this;
        break;
    default:
        delete this;
        break;
    }
    return POLLER_COMPLETE;
}

int CHandlerTcpSocket::handle_output()
{
    LogMsg("handle_output\n");
    if (m_w.data_len() != 0)
    {
        int ret = ::send(m_net_fd, m_w.data(), m_w.data_len(), 0);
        if (-1 == ret)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EINPROGRESS)
            {
                enable_output();
                apply_events();
                m_status = ECONNECT_DATA_SENDING;
                return m_status;
            }
            disable_input();
            disable_output();
            apply_events();
            m_status = ECONNECT_SEND_ERROR;
            return m_status;
        }
        if (ret == m_w.data_len())
        {
            disable_output();
            enable_input();
            apply_events();
            m_w_skip_idx = ret;
            m_w.skip(m_w_skip_idx);
            m_status = ECONNECT_SEND_DONE;
            return m_status;
        }
        else if (ret < m_w.data_len())
        {
            enable_output();
            apply_events();
            m_w_skip_idx = ret;
            m_w.skip(m_w_skip_idx);
            m_status = ECONNECT_DATA_SENDING;
            return m_status;
        }
    }
    disable_output();
    apply_events();
    m_status = ECONNECT_SEND_ERROR;
    return m_status;
}

void CHandlerTcpSocket::append_write(const char *src, int len)
{
    LogMsg("append_write \n");
    m_w.append(src, len);
}

int CHandlerTcpSocket::send_msg(PBMessage &message)
{
    CPBOutputPacker output_packer;
    if (!output_packer.encode_proto_msg(message))
    {
        ErrMsg("output_packer.encode_proto_msg error \n");
        print_message(message);
        return -1;
    }
    output_packer.end();
    int ret = 0;
    if (-1 == (ret = send(output_packer.packet_buf(), output_packer.packet_size())))
    {
        ErrMsg("send error [%s]\n", strerror(errno));
        return -1;
    }
    else
    {
        LogMsg("send [%s],[%d]\n", output_packer.packet_buf(), ret);
        return 0;
    }
}

void CHandlerTcpSocket::print_message(PBMessage &message)
{
    std::string strProto;
    google::protobuf::TextFormat::PrintToString(message, &strProto);
    LogMsg("message id[%d]\n", message.msg_case());
    LogMsg("message [%s]\n", strProto.c_str());
}

bool CHandlerTcpSocket::need_process_wait_message()
{
    return !m_wait_message.empty();
}

int CHandlerTcpSocket::process_wait_message()
{
    int ret = send_msg(m_wait_message.front());
    m_wait_message.pop_front();
    return ret;
}

void CHandlerTcpSocket::push_wait_message(PBMessage &message)
{
    m_wait_message.push_back(message);
}

bool CHandlerTcpSocket::analysis_data(CCache &data)
{
    int packet_len = 0;
    int ret = 0;
    LogMsg("m_r.length()[%d]\n", m_r.data_len());
    packet_len = m_decoder->unpack(m_r.data(), m_r.data_len());
    if (packet_len == -1)
    {
        // 数据错误
        disable_input();
        m_status = ECONNECT_RECV_ERROR;
        ErrMsg("数据错误 [%s]\n", m_r.data());
        return false;
    }
    else if (packet_len == 0)
    {
        // 包头解析完, 等待包体
        LogMsg("包头解析完 等待包体 [%s]\n", m_r.data());
        m_status = ECONNECT_DATA_RECEIVING;
        return false;
    }
    else
    {
        // 解析到完整的包
        int fd = get_net_fd();
        LogMsg("解析到完整的包 [%s]\n", m_r.data());
        ret = on_packet_complete(m_r.data(), packet_len);
        if (ret < 0)
        {
            m_status = ECONNECT_RECV_ERROR;
            ErrMsg("handler[%d] on packet complete . err\n", fd);
            return false;
        }
        m_status = ECONNECT_RECV_DONE;
        m_r_skip_idx = packet_len;
        m_r.skip(m_r_skip_idx);
        return true;
    }

}