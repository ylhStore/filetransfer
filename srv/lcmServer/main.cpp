//
// Created by ylh on 2021/5/26.
//

#include <signal.h>

#include "NewReactor.h"
#include "ProcessorManager.h"
#include "Tool.h"
#include "Config.h"
#include "AppServer.h"
#include "LCMTcpqHandler.h"

long long g_sig_flag = 0;

// 做LCM tcpq服务
class CLCMTcpqHandlerProxy : public CHandlerProxyBase, public CSingleton<CLCMTcpqHandlerProxy>
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
CPacketDecoder CLCMTcpqHandlerProxy::stat_decoder;
CPacketEncoder CLCMTcpqHandlerProxy::stat_encoder;

int run_echo()
{
    // 初始化 处理器
    CProcessManager::instance()->init();

    // 初始化 服务端 LCM tcpq
    CAppServer<CLCMTcpqHandler> server(CLCMTcpqHandlerProxy::instance(), DEFAULT_ACCEPT_CNT, DEFAULT_BACK_LOG);

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

int main()
{
    // return run_normal();
    return run_echo();
}
