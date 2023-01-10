//
// Created by ylh on 2021/6/2.
//
#include <google/protobuf/text_format.h>
#include "HandlerTransmission.h"

CHandlerTransmission::CHandlerTransmission(CHandlerProxyBase *HandlerProxy, CServerTransfer *control_handler) :
        CHandlerTcpSocket()
{
    m_control_handler = control_handler;
    m_handler_proxy = HandlerProxy;
    m_file_info = SFileInfo(control_handler->m_transfer_file_name);
    m_transfer_file_name = control_handler->m_transfer_file_name;
}

CHandlerTransmission::~CHandlerTransmission()
{
    reset_file_info();
    LogMsg("~CHandlerTransmission\n");
    if (m_control_handler)
    {
        m_control_handler->m_transfer_file_name = "";
        delete m_control_handler;
        m_control_handler = nullptr;
    }
}

void CHandlerTransmission::reset_file_info()
{
    if (m_file_info.m_fd > 0)
    {
        close(m_file_info.m_fd);
        m_file_info.m_fd = -1;
    }
}

CHandlerTransmission::SFileInfo &CHandlerTransmission::get_file_info()
{
    return m_file_info;
}

CHandlerTransmission::SFileInfo *CHandlerTransmission::mutable_file_info()
{
    return &m_file_info;
}

