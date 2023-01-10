//
// Created by ylh on 2021/6/8.
//
#include "ProcessorManager.h"
#include "Processor.h"

void CProcessManager::init()
{
    REGISTER_PROCESSOR(PBMessage::kRequestGet, CRequestGet);
    REGISTER_PROCESSOR(PBMessage::kRequestList, CRequestList);
    REGISTER_PROCESSOR(PBMessage::kRequestPasv,CRequestPasv);
    REGISTER_PROCESSOR(PBMessage::kRequestDownload,CRequestDownload);
}

CProcessorBase *CProcessManager::get_processor(int msg_id)
{
    ProcessorMap::iterator iter = m_processor_map.find(msg_id);
    if (m_processor_map.end() == iter)
    {
        return nullptr;
    } else
    {
        return iter->second;
    }
}

void CProcessManager::process(CHandlerTcpSocket *handler, const PBMessage &msg)
{
    CProcessorBase* processor = get_processor(msg.msg_case());
    if(nullptr == processor)
    {
        ErrMsg("error not find processor,msg_case[%d]\n",msg.msg_case());
    }else{
        processor->process(handler,msg);
    }
}
