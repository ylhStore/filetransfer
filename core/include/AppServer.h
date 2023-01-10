#include "ServerBase.h"
template <typename T>
class CAppServer : public CServerBase
{
private:
public:
    CAppServer(CHandlerProxyBase *HandlerProxy, int accept_cnt, int backlog)
        : CServerBase(HandlerProxy, accept_cnt, backlog)
    {
    }
    virtual ~CAppServer(){};

    virtual CHandlerTcpSocket *CreateHandler() override
    {
        return new T();
    }
};