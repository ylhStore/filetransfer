#include "LCMTcpqHandler.h"
#define MAGIC_SERVER 0x287617fa  // first word sent by server
#define MAGIC_CLIENT 0x287617fb  // first word sent by client
#define PROTOCOL_VERSION 0x0100  // what version do we implement?

CLCMTcpqHandler::CLCMTcpqHandler()
{
    recv_func = new recv_func_point[E_LCM_TCPQ_HADNLER_MAX];
    memset(recv_func, 0, sizeof(recv_func_point) * E_LCM_TCPQ_HADNLER_MAX);
    recv_func[E_LCM_TCPQ_HADNLER_INIT] = &CLCMTcpqHandler::recv_in_init;
    recv_func[E_LCM_TCPQ_HADNLER_TRANSFER] = &CLCMTcpqHandler::recv_in_transfer;

    handler_state = E_LCM_TCPQ_HADNLER_INIT;
}

EConnStatus CLCMTcpqHandler::handle_input()
{
    if (recv_func[handler_state])
    {
        return (this->*recv_func[handler_state])();
    }
    else
    {
        return ECONNECT_RECV_ERROR;
    }
}

EConnStatus CLCMTcpqHandler::recv_in_init()
{
    int curr_recv_len = 0;
    char curr_recv_buf[MAX_RECV_LEN];
    memset(curr_recv_buf, 0, sizeof(curr_recv_buf));

    SCheckData check_data;
    int recv_aim_len = sizeof(check_data);

    int len = 0;
    while (curr_recv_len < recv_aim_len)
    {
        len = recv(m_net_fd, curr_recv_buf + curr_recv_len, recv_aim_len - curr_recv_len, 0);
        if (-1 == len)
        {
            if (errno != EAGAIN && errno != EINTR && errno != EINPROGRESS)
            {
                disable_input();
                m_status = ECONNECT_RECV_ERROR;
                ErrMsg("recv failed from fd[%d], msg[%s]\n", m_net_fd, strerror(errno));
                return m_status;
            }
            else
            {
                m_status = ECONNECT_DATA_RECEIVING;
                return m_status;
            }
        }
        else if (0 == curr_recv_len)
        {
            disable_input();
            m_status = ECONNECT_DISCONNECT;
            LogMsg("connection disconnect by user fd[%d],msg[%s]\n", m_net_fd, strerror(errno));
            return m_status;
        }
        else
        {
            curr_recv_len += len;
        }
    }

    m_input_packer.reset();
    check_data.magic_client = m_input_packer.read_uint32();
    check_data.protocal_version = m_input_packer.read_uint32();

    if(check_data.magic_client != MAGIC_CLIENT || check_data.protocal_version != PROTOCOL_VERSION)
    {
        m_status =  ECONNECT_RECV_ERROR;
        return m_status;
    }

    m_output_packer._reset();
    m_output_packer.write_uint32(MAGIC_SERVER);
    m_output_packer.write_uint32(PROTOCOL_VERSION);

    m_w.append(m_output_packer.packet_buf(),m_output_packer.packet_size());

    disable_input();
    enable_output();
    
    m_status = ECONNECT_RECV_DONE;
    return m_status;
}

EConnStatus CLCMTcpqHandler::recv_in_transfer()
{
    int curr_recv_len = 0;
    char curr_recv_buf[MAX_RECV_LEN];
    memset(curr_recv_buf, 0, sizeof(curr_recv_buf));

    

    m_status = ECONNECT_RECV_DONE;
    return m_status;
}






