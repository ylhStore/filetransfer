//
// Created by ylh on 2021/6/2.
//

#include "Poller.h"
#include "Log.h"

//// 定义 CPollerObj 内容
CPollerObj::CPollerObj(CPollerManager *owner, int fd) : m_net_fd(fd),
                                                        newEvents(0),
                                                        oldEvents(0),
                                                        m_owner_unit(owner),
                                                        m_poller_node(nullptr)
{
}

CPollerObj::~CPollerObj()
{
    if (m_owner_unit && m_poller_node)
    {
        m_owner_unit->free_poller(m_poller_node);
    }
    if (m_net_fd > 0)
    {
        close(m_net_fd);
        m_net_fd = 0;
    }
}

int CPollerObj::attach_poller(CPollerManager *unit)
{
    if (unit)
    {
        if (nullptr == m_owner_unit)
        {
            m_owner_unit = unit;
        }
    }
    if (m_net_fd < 0)
    {
        return -1;
    }
    if (nullptr == m_poller_node)
    {
        if (!(m_poller_node = m_owner_unit->alloc_poller()))
        {
            return -1;
        }
        m_poller_node->m_data_ptr = this;
        int flag = fcntl(m_net_fd, F_GETFL);
        fcntl(m_net_fd, F_SETFL, O_NONBLOCK | flag);
        epoll_event ev;
        ev.events = newEvents;
        ev.data.ptr = m_poller_node;
        if (0 == m_owner_unit->epoll_ctl(EPOLL_CTL_ADD, m_net_fd, &ev))
        {
            oldEvents = newEvents;
        }
        else
        {
            return -1;
        }
        return 0;
    }
    else
    {
        return apply_events();
    }
}

int CPollerObj::detach_poller()
{
    if (m_poller_node)
    {
        epoll_event ev;
        if (0 == m_owner_unit->epoll_ctl(EPOLL_CTL_DEL, m_net_fd, &ev))
        {
            oldEvents = newEvents;
        }
        else
        {
            return -1;
        }
        m_owner_unit->free_poller(m_poller_node);
        m_poller_node = nullptr;
    }
    return 0;
}

int CPollerObj::apply_events()
{
    if (nullptr == m_poller_node)
    {
        return 0;
    }
    if (oldEvents == newEvents)
    {
        return 0;
    }
    epoll_event ev;
    ev.events = newEvents;
    ev.data.ptr = m_poller_node;
    if (0 == m_owner_unit->epoll_ctl(EPOLL_CTL_MOD, m_net_fd, &ev))
    {
        oldEvents = newEvents;
    }
    else
    {
        return -1;
    }
    return 0;
}

int CPollerObj::process_input()
{
    modify_input(false);
    return POLLER_SUCCESS;
}

int CPollerObj::process_output()
{
    modify_output(false);
    return POLLER_SUCCESS;
}

int CPollerObj::process_hang_up()
{
    delete this;
    return POLLER_FAIL;
}

//// 定义 CPollerManager 内容

CPollerManager::CPollerManager(int mp)
{
    m_max_pollers = mp;
    m_epoll_fd = -1;
    m_ep_events = nullptr;
    m_used_pollers = 0;
}

CPollerManager::~CPollerManager()
{
    if (nullptr != m_ep_events)
    {
        delete[] m_ep_events;
        m_ep_events = nullptr;
    }
    if (-1 != m_epoll_fd)
    {
        close(m_epoll_fd);
        m_epoll_fd = -1;
    }
}

int CPollerManager::set_max_pollers(int mp)
{
    if (m_epoll_fd >= 0)
    {
        return -1;
    }
    m_max_pollers = mp;
    return 0;
}

int CPollerManager::init_poller_unit()
{
    if (!init_m_pollers())
    {
        ErrMsg("init_poller_unit, new pollers error [num:%d]\n", m_max_pollers);
        return -1;
    }

    m_ep_events = new epoll_event[m_max_pollers];
    if (!m_ep_events)
    {
        ErrMsg("init_poller_unit, new epoll_event error [num:%d]\n", m_max_pollers);
        return -1;
    }
    if (-1 == (m_epoll_fd = epoll_create(m_max_pollers)))
    {
        ErrMsg("init_poller_unit, epoll_create error [%s] ", strerror(errno));
        return -1;
    }
    return 0;
}

bool CPollerManager::init_m_pollers()
{
    return m_pollers.init(m_max_pollers);
}

void CPollerManager::free_poller(SNode<CPollerObj> *poller_packet)
{
    --m_used_pollers;
    m_pollers.free_node(poller_packet);
}

SNode<CPollerObj> *CPollerManager::alloc_poller()
{
    ++m_used_pollers;
    return m_pollers.alloc_node();
}

int CPollerManager::epoll_ctl(int op, int fd, epoll_event *ev)
{
    if (-1 == ::epoll_ctl(m_epoll_fd, op, fd, ev))
    {
        ErrMsg("epoll_ctl error [%s]", strerror(errno));
        return -1;
    }
    return 0;
}

bool CPollerManager::wait_poller_events(int timeout)
{
    m_wait_nums = epoll_wait(m_epoll_fd, m_ep_events, m_max_pollers, timeout);
    if (m_wait_nums < 0)
    {
        if (errno != EINTR)
        {
            ErrMsg("wait_poller_events error [%s]\n", strerror(errno));
        }
        return false;
    }
    return true;
}

void CPollerManager::process_poller_events()
{
    int ret_code = -1;
    for (int idx = 0; idx < m_wait_nums; ++idx)
    {
        SNode<CPollerObj> *s_node = (SNode<CPollerObj> *)m_ep_events[idx].data.ptr;
        CPollerObj *poller_obj = s_node->m_data_ptr;
        poller_obj->newEvents = poller_obj->oldEvents;

        if (m_ep_events[idx].events & (EPOLLERR | EPOLLHUP))
        {
            ErrMsg("EPOLLHUP | EPOLLERR,events[%d] netfd[%d]\n",m_ep_events[idx].events,poller_obj->m_net_fd);
            poller_obj->process_hang_up();
            continue;
        }

        if (m_ep_events[idx].events & EPOLLIN)
        {
            ret_code = poller_obj->process_input();
            // 因为可能在 process_input 中删除自己 所以这里需要判断是否已经删除
            if (s_node->m_data_ptr != poller_obj)
            {
                continue;
            }

            // 再次检测返回值
            if (POLLER_CONTINUE == ret_code)
            {
                continue;
            }
        }
        if (m_ep_events[idx].events & EPOLLOUT)
        {
            ret_code = poller_obj->process_output();
            // 因为可能在 process_input 中删除自己 所以这里需要判断是否已经删除
            if (s_node->m_data_ptr != poller_obj)
            {
                continue;
            }
            if (POLLER_CONTINUE == ret_code)
            {
                continue;
            }
        }
        poller_obj->apply_events();
    }
}
