//
// Created by ylh on 2021/6/8.
//

#ifndef CENTOS6_9_MINI_PROCESSERMANAGER_H
#define CENTOS6_9_MINI_PROCESSERMANAGER_H

#include <map>
#include "Singleton.h"
#include "Processor.h"

#define REGISTER_PROCESSOR(msg_id, processor) \
do{                                          \
    CProcessorBase* m_processor = new processor(); \
    m_processor_map[msg_id] = m_processor;\
}while(false)


/*
 * @ brief 消息处理器管理类 绑定消息与消息处理器以及提供外部调用接口
 * */
class CProcessManager : public CSingleton<CProcessManager>
{
private:
    typedef std::map<int, CProcessorBase *> ProcessorMap;
    ProcessorMap m_processor_map; // 管理消息与其对应的处理器
public:
    /*
     * @ brief 初始化 消息与其对应的处理器
     * */
    void init();

    /*
     * @ brief 获得消息对应的处理器
     * @ param msg_id 消息id
     * @ return 处理器
     * */
    CProcessorBase *get_processor(int msg_id);

    /*
     * @ brief 处理消息
     * @ param handler 等待处理消息的连接
     * @ param message 等待处理的消息
     * */
    void process(CHandlerTcpSocket* handler,const PBMessage& msg);
};

#endif //CENTOS6_9_MINI_PROCESSERMANAGER_H
