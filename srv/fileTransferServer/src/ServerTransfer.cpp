//
// Created by ylh on 2021/6/11.
//
#include "ServerTransfer.h"
#include "HandlerTransmission.h"
#include "NewReactor.h"

CServerTransfer::~CServerTransfer()
{
    if (nullptr != m_control_handler)
    {
        m_control_handler->m_transfer_handler = nullptr;
        m_control_handler = nullptr;
    }
    if(CNewReactor::instance()->exist_adapter(m_name))
    {
        CNewReactor::instance()->delete_adapter(m_name);
    }
}

int CServerTransfer::process_request(sockaddr *addr)
{
    for (int idx = 0; idx < m_new_fd_cnt; ++idx)
    {
        if (-1 == m_fd_array[idx])
        {
            continue;
        }
        CHandlerTransmission *tmp_tcp_handler = new CHandlerTransmission(m_handler_proxy,this);
        tmp_tcp_handler->set_sock_addr(&m_peer_array[idx]);
        tmp_tcp_handler->set_net_fd(m_fd_array[idx]);
        if (CNewReactor::instance()->register_client(tmp_tcp_handler) < 0)
        {
            m_fd_array[idx] = -1;
            delete tmp_tcp_handler;
            tmp_tcp_handler = nullptr;
            continue;
        }
    }
    return 0;
}
