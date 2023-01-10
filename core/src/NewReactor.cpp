//
// Created by ylh on 2021/6/2.
//
#include "NewReactor.h"
#include "Log.h"

extern long long g_sig_flag;

void signal_handler(int sig)
{
    g_sig_flag = 1;
}

bool CNewReactor::check_sig_flag()
{
    return g_sig_flag == FLAG_RUN;
}

bool CNewReactor::init(int max_poller)
{
    m_max_poller = max_poller;
    m_poller_unit = new CPollerManager(m_max_poller);
    if (m_poller_unit->init_poller_unit() < 0)
    {
        return false;
    }

    //挂载信号
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);

    return true;
}

bool CNewReactor::bind_server(CServerBase *adapter)
{
    if (nullptr == adapter)
    {
        return false;
    }
    if (m_servers.find(adapter->get_name()) == m_servers.end())
    {
        if (!adapter->bind())
        {
            return false;
        }
        if (0 != attach_poller(adapter))
        {
            return false;
        }
        m_servers.insert(std::make_pair(adapter->get_name(), adapter));
    } else
    {
        return false;
    }
    return true;
}

int CNewReactor::register_client(CPollerObj *Poller)
{
    return attach_poller(Poller);
}

int CNewReactor::attach_poller(CPollerObj *Poller)
{
    if (Poller->on_attach_poller() != 0)
    {
        ErrMsg("Poller attach failed\n");
        return -1;
    }
    return Poller->attach_poller(m_poller_unit);
}

int CNewReactor::run_event_loop()
{
    while (true)
    {
        if (!check_sig_flag())
        {
            printf("stop run\n");
            break;
        }

        if(m_poller_unit->get_used_pollers() <= 0)
        {
            printf("used_pollers <= 0, stop run\n");
            break;
        }

        m_poller_unit->wait_poller_events(1000);
        m_poller_unit->process_poller_events();
    }
    return 0;
}

bool CNewReactor::exist_adapter(std::string &name)
{
    return m_servers.find(name) != m_servers.end();
}

void CNewReactor::delete_adapter(std::string &name)
{
    m_servers.erase(name);
}



