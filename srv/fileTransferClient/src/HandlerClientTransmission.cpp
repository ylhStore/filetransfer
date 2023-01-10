//
// Created by ylh on 2021/6/3.
//
#include <google/protobuf/text_format.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "HandlerClientTransmission.h"
#include "Tool.h"

CHandlerClientTransmission::CHandlerClientTransmission(CHandlerProxyBase *HandlerProxy, CHandlerClient *control_handler)
{
    m_server_port = 0;
    m_name = "";
    m_handler_proxy = HandlerProxy;
    //数据链路 绑定 控制链路
    m_control_handler = control_handler;
    memset(m_server_addr, 0, sizeof(m_server_addr));
    m_start_func = nullptr;
    m_file_info = SFileInfo(control_handler->get_aim_file_name());

    m_time_start = m_tool::getCurrentTime();
    m_time_end = m_time_start;

}

CHandlerClientTransmission::~CHandlerClientTransmission()
{
    LogMsg("~CHandlerClientTransmission\n");
    reset_file_info();
    if (m_control_handler)
    {
        m_control_handler->reset_aim_file_name();
        m_control_handler->m_transmission_handler = nullptr;
        m_control_handler = nullptr;
    }
    m_time_end = m_tool::getCurrentTime();
    printf("cost time [%ld]\n",m_time_end - m_time_start);
}

void CHandlerClientTransmission::init(const std::string &name, const char *ip, uint16_t port, Func func)
{
    m_name = name;
    m_server_port = port;
    strncpy(m_server_addr, ip, sizeof(m_server_addr) - 1);
    m_start_func = func;
}

int CHandlerClientTransmission::process_input()
{
    LogMsg("CHandlerClientTransmission process_input\n");
    int stage = handle_input();

    // 判断是否需要关闭 链接 , 在消息处理之后 可能要需要关闭连接
    if(get_need_close())
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

int CHandlerClientTransmission::process_output()
{
    if (nullptr == m_start_func)
    {
        delete this;
        return POLLER_FAIL;
    }
    LogMsg("process_output\n");

    /*
     * 如果处理了等待发送的消息 则本次事件响应结束
     * */
    if(need_process_wait_message())
    {
        if(-1 == process_wait_message())
        {
            delete this;
            return POLLER_COMPLETE;
        }else{

            // 判断是否需要关闭 链接 , 在消息处理之后 可能要需要关闭连接 但是处理消息的地方 和 发送消息的地方不在一起
            // 所以 在处理消息的地方 只是设置是否关闭 在这里发送完消息之后 才关闭
            if(get_need_close())
            {
                delete this;
                return POLLER_COMPLETE;
            }

            disable_output();
            enable_input();
            return POLLER_SUCCESS;
        }
    }

    // 判断是否需要关闭 链接 , 在消息处理之后 可能要需要关闭连接 但是处理消息的地方 和 发送消息的地方不在一起
    // 所以 在处理消息的地方 只是设置是否关闭 在这里发送完消息之后 才关闭
    if(get_need_close())
    {
        delete this;
        return POLLER_COMPLETE;
    }

    int stage = (this->*m_start_func)();
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

void CHandlerClientTransmission::reset_file_info()
{
    if (m_file_info.m_fd > 0)
    {
        close(m_file_info.m_fd);
    }
}

CHandlerClientTransmission::SFileInfo CHandlerClientTransmission::get_file_info()
{
    return m_file_info;
}

CHandlerClientTransmission::SFileInfo *CHandlerClientTransmission::mutable_file_info()
{
    return &m_file_info;
}

//------------------------------------定义 能够是start_func的函数--- start
int CHandlerClientTransmission::transfer_file()
{
    PBMessage message;
    RequestDownload *request_download = message.mutable_request_download();
    int offset = m_file_info.m_offset;
    int fd = m_file_info.m_fd;
    if (fd < 0)
    {
        // 文件没有记录过 需要测试是否是断点传输
        fd = open(m_file_info.m_file_name.c_str(), O_EXCL);
        if (fd < 0)
        {
            // 文件不存在 则不是断点传输
            request_download->set_status(E_DOWNLOAD_LUNCH);
            offset = 0;
        }
        else
        {
            // 文件存在 是断点传输
            request_download->set_status(E_DOWNLOAD_ING);
            struct stat st;
            if (-1 == stat(m_file_info.m_file_name.c_str(), &st))
            {
                ErrMsg("获取文件状态失败\n [%s]", m_file_info.m_file_name.c_str());
                close(fd);
                return POLLER_FAIL;
            }
            offset = st.st_size;
        }
        fd = open(m_file_info.m_file_name.c_str(), O_WRONLY | O_CREAT);
        if (fd < 0)
        {
            ErrMsg("open file[%s] failed, errno[%s]\n", m_file_info.m_file_name.c_str(), strerror(errno));
            return POLLER_FAIL;
        }
        m_file_info.m_fd = fd;
        m_file_info.m_offset = offset;
    }
    else
    {
        // 文件有记录过 offset 应该是要开始传输的位置
        offset = offset;
    }

    request_download->set_buffer_size(MAX_TRANS_SIZE);
    request_download->set_offset(offset);

    if (send_msg(message) < 0)
    {
        return POLLER_FAIL;
    }

    print_message(message);

    return POLLER_SUCCESS;
}

//------------------------------------定义 能够是start_func的函数--- end
