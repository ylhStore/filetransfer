#include "TcpSocketHandler.h"
#include "LCMTcpqPacket.h"

enum E_LCM_TCPQ_HADNLER_STATE
{
    E_LCM_TCPQ_HADNLER_INIT = 0,     // 连接初始化阶段 接收 发送数据包版本数据
    E_LCM_TCPQ_HADNLER_TRANSFER = 1, // 数据传输阶段
    E_LCM_TCPQ_HADNLER_MAX = 2       // 状态最大值
};

class CLCMTcpqHandler : public CHandlerTcpSocket
{
private:
    struct LCMTcpqCheckData // 需要校验的数据
    {
        uint32_t magic_client;
        uint32_t protocal_version;
    };
    typedef LCMTcpqCheckData SCheckData;

    int protocal_version;                                      // 协议版本
    E_LCM_TCPQ_HADNLER_STATE handler_state;                    // 标志当前是哪个阶段
    typedef EConnStatus (CLCMTcpqHandler::*recv_func_point)(); // 定义recv处理函数格式
    recv_func_point *recv_func;                                // recv处理函数数组
    CLCMTcpqInputPacker m_input_packer;                        // 基础数据解析工具
    CLCMTcpqOutputPacker m_output_packer;                      // 基础数据解析工具

public:
    CLCMTcpqHandler();
    virtual ~CLCMTcpqHandler() {}

    virtual EConnStatus handle_input();

    /*
     * @ brief 初始化阶段接收数据
     * @ return 返回处理结果
    */
    EConnStatus recv_in_init();
    /*
     * @ brief 数据传输阶段接收数据
     * @ return 返回处理结果
    */
    EConnStatus recv_in_transfer();
};