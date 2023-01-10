//
// Created by ylh on 2021/6/3.
//
#include "HandlerClient.h"
#include <google/protobuf/text_format.h>
#include <termios.h>

CHandlerClient::CHandlerClient(CHandlerProxyBase *HandlerProxy)
{
    m_server_port = 0;
    m_name = "";
    m_handler_proxy = HandlerProxy;
    memset(m_server_addr, 0, sizeof(m_server_addr));
    m_transmission_handler = nullptr;
}

CHandlerClient::~CHandlerClient()
{
}

int CHandlerClient::process_input()
{
    int stage = handle_input();

    // 判断是否需要关闭 链接 , 在消息处理之后 可能要需要关闭连接
    if (get_need_close())
    {
        delete this;
        return POLLER_COMPLETE;
    }

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

int CHandlerClient::process_output()
{
    // 有数据链路在传输 不监听终端输入
    if (nullptr != m_transmission_handler)
    {
        return POLLER_SUCCESS;
    }

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
     * 没有等待发送的消息处理 则响应终端输入
     * */
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
    case POLLER_FAIL:
        delete this;
        return POLLER_FAIL;
    case POLLER_SUCCESS:
    case POLLER_COMPLETE:
        disable_output();
        enable_input();
        break;
    case POLLER_CONTINUE:
        return POLLER_CONTINUE;
    default:
        delete this;
        return POLLER_FAIL;
    }
    return POLLER_SUCCESS;
}

int CHandlerClient::handle_output()
{
    std::string cmd;
    std::string body;
    printf(">> ");
    terminal_input(cmd, body);
    PBMessage message;
    if (fill_request_message(cmd, body, message) < 0)
    {
        return POLLER_CONTINUE;
    }
    if (send_msg(message) < 0)
    {
        return POLLER_FAIL;
    }
    return POLLER_SUCCESS;
}

int CHandlerClient::terminal_input(std::string &cmd, std::string &body)
{
    char buffer[MAX_RECV_LEN];
    memset(buffer, 0, sizeof(buffer));
    int idx = -1;
    int len = 0;
    int space_idx = -1;
    int space_nums = 0;
    char ch;
    // 处理终端 终端输入
    {
        int max_idx = MAX_RECV_LEN - 1;

        termios info;
        tcgetattr(0, &info);
        info.c_lflag &= ~ECHO;
        info.c_lflag &= ~ICANON;

        tcsetattr(0, TCSANOW, &info);

        for (idx = -1; idx < max_idx;)
        {
            ch = getchar();

            if ('\n' == ch)
            {
                break;
            }
            if ('\b' == ch)
            {
                // 处理退格号
                putchar(ch);
                if (idx >= 0)
                {
                    if (' ' == buffer[idx])
                    {
                    }
                    buffer[idx--] = 0;
                }
                continue;
            }
            buffer[++idx] = ch;
            putchar(ch);
            if (' ' == ch)
            {
                if (++space_nums <= 1)
                {
                    space_idx = idx;
                }
            }
        }

        info.c_lflag |= ECHO;
        info.c_lflag |= ICANON;

        tcsetattr(0, TCSANOW, &info);

        len = idx;
    }

    // 分割输入内容
    if (space_idx == -1)
    {
        char str_cmd[MAX_RECV_LEN];
        memset(str_cmd, 0, sizeof(str_cmd));
        strncpy(str_cmd, buffer, len + 1);
        cmd = str_cmd;
    }
    else
    {
        char str_cmd[MAX_RECV_LEN];
        memset(str_cmd, 0, sizeof(str_cmd));
        strncpy(str_cmd, buffer, space_idx);
        cmd = str_cmd;

        memset(str_cmd, 0, sizeof(str_cmd));
        strncpy(str_cmd, buffer + space_idx + 1, len - space_idx);
        body = str_cmd;
    }
    printf("\n");
    return 0;
}

int CHandlerClient::fill_request_message(std::string &cmd, std::string &body, PBMessage &message)
{
    if (cmd.compare("list") == 0)
    {
        RequestList list;
        message.mutable_request_list()->CopyFrom(list);
    }
    else if (cmd.compare("get") == 0)
    {
        if (body.empty())
        {
            ErrMsg("cmd == get but body is empty\n");
            return -1;
        }
        RequestGet get;
        get.set_file_path(body);
        message.mutable_request_get()->CopyFrom(get);
        set_aim_file_name(body);
    }
    else
    {
        ErrMsg("unknown cmd[%s]\n", cmd.c_str());
        return -1;
    }
    print_message(message);
    return 0;
}

std::string CHandlerClient::get_aim_file_name()
{
    return m_aim_file_name;
}
