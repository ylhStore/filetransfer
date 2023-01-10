//
// Created by ylh on 2021/5/26.
//
#include <signal.h>
#include "Tool.h"
#include "TcpSocketHandler.h"
#include "NewReactor.h"
#include "HandlerClient.h"
#include "ClientHandlerProxy.h"
#include "ProcessorManager.h"
#include "Config.h"

// #define PORT 5000
// #define IP "127.0.0.1"

long long g_sig_flag = 0;

// 做反射服务
class CHandlerEcho : public CHandlerTcpSocket
{
public:
    unsigned int m_max_run_times; // 最大运行次数
    unsigned int m_run_times;     // 运行次数
public:
    CHandlerEcho()
    {
        m_run_times = 0;
        m_max_run_times = 0;
    }
    CHandlerEcho(CHandlerProxyBase *HandlerProxy, unsigned int max_run_times, unsigned int run_times = 0) : m_max_run_times(max_run_times),
                                                                                                            m_run_times(run_times)

    {
        m_handler_proxy = HandlerProxy;
    }
    virtual ~CHandlerEcho()
    {
    }
    virtual int process_output() override;
    virtual int on_packet_complete(const char *data, int len) override;
    bool need_close() { return m_run_times >= m_max_run_times; }
};

int CHandlerEcho::process_output()
{
    ++m_run_times;

    char put[] = "hello\n";
    int len = strlen(put);
    COutputPacket output_packer;
    output_packer.write_binary(put, len);
    output_packer.end();
    int packet_size = output_packer.packet_size();
    append_write(output_packer.packet_buf(), output_packer.packet_size());

    int stage = handle_output();
    bool will_close = false;
    if (get_need_close())
    {
        will_close = true;
    }

    if (need_close())
    {
        will_close = true;
    }

    switch (stage)
    {
    case ECONNECT_SEND_DONE:
        if (will_close)
        {
            delete this;
            return POLLER_SUCCESS;
        }
        return POLLER_COMPLETE;
    case ECONNECT_DATA_SENDING:
        return POLLER_SUCCESS;
    case ECONNECT_SEND_ERROR:
        delete this;
        return POLLER_COMPLETE;
    default:
        delete this;
        return POLLER_COMPLETE;
    }
}

int CHandlerEcho::on_packet_complete(const char *data, int len)
{
    disable_output();
    enable_output();
    return 0;
}

int run_echo(int client_num, int single_run_time)
{
    if (!CNewReactor::instance()->init())
    {
        printf("CNewReactor::instance()->init() error\n");
        return -1;
    }
    for (int idx = 0; idx < client_num; ++idx)
    {
        CHandlerEcho *client = new CHandlerEcho(CClientHandlerProxy::instance(), single_run_time);
        client->init("client" + std::to_string(idx), DEFAULT_IP, DEFAULT_PORT);
        if (!client->connect())
        {
            printf("client connect error[%s]\n", strerror(errno));
            return -1;
        }
        if (CNewReactor::instance()->register_client(client) < 0)
        {
            printf("CNewReactor::instance()->register_client error\n");
            return -1;
        }
    }
    long int start, end;
    start = m_tool::getCurrentTime();
    CNewReactor::instance()->run_event_loop();
    end = m_tool::getCurrentTime();
    printf("cost [%ld]\n", end - start);
    return 0;
}

int run_normal()
{
    CProcessManager::instance()->init();
    CHandlerClient client(CClientHandlerProxy::instance());
    client.init("client", DEFAULT_IP, DEFAULT_PORT);
    if (!client.connect())
    {
        printf("client connect error[%s]\n", strerror(errno));
        return -1;
    }

    if (!CNewReactor::instance()->init())
    {
        printf("CNewReactor::instance()->init() error\n");
        return -1;
    }

    if (CNewReactor::instance()->register_client(&client) < 0)
    {
        printf("CNewReactor::instance()->register_client error\n");
        return -1;
    }
    CNewReactor::instance()->run_event_loop();
    return 0;
}

constexpr const char *ECHO_ARG = "ECHO";
int main(int argc, char *argv[])
{
    if (argc == 4 && strcmp(argv[1], ECHO_ARG) == 0)
    {
        printf("echo\n");
        int client_num, single_run_time;
        client_num = strtol(argv[2],nullptr,10);
        single_run_time = strtol(argv[3],nullptr,10);
        return run_echo(client_num, single_run_time);
    }
    else
    {
        printf("normal\n");
        return run_normal();
    }
}
