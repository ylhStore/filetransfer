//
// Created by ylh on 2021/6/2.
//
#include "ServerBase.h"
#include "NewReactor.h"

CServerBase::CServerBase(CHandlerProxyBase *HandlerProxy, int accept_cnt, int backlog)
{
    m_accept_cnt = accept_cnt;
    m_back_log = backlog;
    m_new_fd_cnt = 0;
    m_peer_array = new sockaddr[m_accept_cnt];
    m_fd_array = new int[m_accept_cnt];
    memset(m_fd_array, -1, sizeof(int) * m_accept_cnt);
    m_handler_proxy = HandlerProxy;
}

CServerBase::~CServerBase()
{
    delete[] m_fd_array;
    m_fd_array = nullptr;
    delete[] m_peer_array;
    m_peer_array = nullptr;
}

void CServerBase::init(const std::string &name, const char *ip, uint16_t port)
{
    m_name = name;
    m_bind_port = port;
    strncpy(m_bind_addr, ip, sizeof(m_bind_addr) - 1);
}

bool CServerBase::bind()
{
    m_net_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == m_net_fd)
    {
        ErrMsg("CServerBase::bind, socket error[%s]", strerror(errno));
        return false;
    }
    if (-1 == fcntl(m_net_fd, F_SETFL, O_NONBLOCK))
    {
        ErrMsg("CServerBase::bind, fcntl error[%s]", strerror(errno));
        return false;
    }
    // 端口复用
    int reuse_addr = 1;
    setsockopt(m_net_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_bind_port);
    addr.sin_addr.s_addr = inet_addr(m_bind_addr);

    if (-1 == ::bind(m_net_fd, (sockaddr *) &addr, sizeof(addr)))
    {
        close(m_net_fd);
        ErrMsg("bind error[%s] \n", strerror(errno));
        return false;
    }

    if (-1 == listen(m_net_fd, m_back_log))
    {
        close(m_net_fd);
        ErrMsg("listen error[%s] \n", strerror(errno));
        return false;
    }

    CPollerObj::enable_input();

    return true;
}

const std::string CServerBase::get_name()
{
    return m_name;
}

CHandlerTcpSocket* CServerBase::CreateHandler()
{
    return new CHandlerTcpSocket();
}

int CServerBase::on_attach_poller()
{
    return 0;
}

int CServerBase::process_input()
{
    sockaddr *tmp_peer = m_peer_array;
    socklen_t peer_size = sizeof(sockaddr);
    while (true)
    {
        int ret = process_accept(tmp_peer, &peer_size);
        if (m_new_fd_cnt <= 0)
        {
            return POLLER_SUCCESS;
        } else
        {
            process_request(tmp_peer);
        }
        if (ret < 0)
        {
            return POLLER_SUCCESS;
        }
    }
}

int CServerBase::process_accept(sockaddr *addr, socklen_t *addr_size)
{
    memset(m_fd_array, -1, sizeof(int) * m_accept_cnt);
    m_new_fd_cnt = 0;
    for (int idx = 0; idx < m_accept_cnt; ++idx)
    {
        int new_fd = ::accept(m_net_fd, (sockaddr *) &m_peer_array[idx], addr_size);

        if (new_fd < 0)
        {
            if (EINTR == errno)
            {
                continue;
            } else if (EAGAIN == errno)
            {
                return -1;
            }
        }
        m_fd_array[idx] = new_fd;
        m_new_fd_cnt++;
        sockaddr_in *tmp = (sockaddr_in *) &m_peer_array[idx];
        LogMsg("accept client, fd[%d],ip[%s],port[%d]\n", new_fd, inet_ntoa(tmp->sin_addr), ntohs(tmp->sin_port));
    }
    return 0;
}

int CServerBase::process_request(sockaddr *addr)
{
    for (int idx = 0; idx < m_new_fd_cnt; ++idx)
    {
        if (-1 == m_fd_array[idx])
        {
            continue;
        }
        CHandlerTcpSocket *tmp_tcp_handler = CreateHandler();
        tmp_tcp_handler->set_sock_addr(&m_peer_array[idx]);
        tmp_tcp_handler->set_net_fd(m_fd_array[idx]);
        tmp_tcp_handler->m_handler_proxy = m_handler_proxy;
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







