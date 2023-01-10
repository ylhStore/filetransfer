//
// Created by ylh on 2021/5/26.
//

#include <signal.h>

#include "NewReactor.h"
#include "ProcessorManager.h"
#include "ServerHandlerProxy.h"
#include "Tool.h"
#include "Config.h"

long long g_sig_flag = 0;

// 做反射服务
class CEchoServerHandlerProxy : public CHandlerProxyBase, public CSingleton<CEchoServerHandlerProxy>
{
public:
    static CPacketDecoder stat_decoder; // 解码数据包
    static CPacketEncoder stat_encoder; // 编码数据包
public:
    /*
     * @ brief 处理连接添加到poller中时要做的事情
     * @ return 是否能够添加到poller中
     * */
    virtual int on_attach_poller(CHandlerTcpSocket *handler) override
    {
        if (nullptr == handler->m_decoder)
        {
            handler->m_decoder = &stat_decoder;
        }
        if (nullptr == handler->m_encoder)
        {
            handler->m_encoder = &stat_encoder;
        }
        handler->enable_input();
        return 0;
    }
    /*
     * @ brief 处理完整的数据包
     * @ param handler 代替的连接
     * @ param data 数据包内容
     * @ param len 数据包长度
     * @ return 返回处理结果
     * */
    virtual int on_packet_complete(CHandlerTcpSocket *handler, const char *data, int len) override
    {
        CInputPacker packer;
        if (!packer.copy(data, len))
        {
            ErrMsg("packer.copy(data, len) error \n");
            return -1;
        }

        handler->append_write(packer.packet_buf(), packer.packet_size());

        handler->disable_input();
        handler->enable_output();

        return 0;
    }
};
CPacketDecoder CEchoServerHandlerProxy::stat_decoder;
CPacketEncoder CEchoServerHandlerProxy::stat_encoder;

int run_normal()
{
    // 初始化 处理器
    CProcessManager::instance()->init();

    // 初始化 服务端
    CServerBase server(CServerHandlerProxy::instance(), DEFAULT_ACCEPT_CNT, DEFAULT_BACK_LOG);

    server.init("server" + m_tool::unique_string_id(), DEFAULT_IP, DEFAULT_PORT);
    if (!CNewReactor::instance()->init())
    {
        printf("CNewReactor::instance()->init() error\n");
        return -1;
    }
    if (!CNewReactor::instance()->bind_server(&server))
    {
        printf("CNewReactor::instance()->bind_server error\n");
        return -1;
    }
    CNewReactor::instance()->run_event_loop();
    return 0;
}

int run_echo()
{
    // 初始化 处理器
    CProcessManager::instance()->init();

    // // 初始化 服务端
    // CServerBase server(CServerHandlerProxy::instance(), DEFAULT_ACCEPT_CNT, DEFAULT_BACK_LOG);

    // 初始化 服务端 做反射服务
    CServerBase server(CEchoServerHandlerProxy::instance(), DEFAULT_ACCEPT_CNT, DEFAULT_BACK_LOG);

    server.init("server" + m_tool::unique_string_id(), DEFAULT_IP, DEFAULT_PORT);
    if (!CNewReactor::instance()->init())
    {
        printf("CNewReactor::instance()->init() error\n");
        return -1;
    }
    if (!CNewReactor::instance()->bind_server(&server))
    {
        printf("CNewReactor::instance()->bind_server error\n");
        return -1;
    }
    CNewReactor::instance()->run_event_loop();
    return 0;
}

constexpr const char *ECHO_ARG = "ECHO";

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], ECHO_ARG) == 0)
    {
        printf("echo\n");
        return run_echo();
    }
    else
    {
        printf("normal\n");
        return run_normal();
    }
}
