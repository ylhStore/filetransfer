//
// Created by ylh on 2021/6/8.
//

#ifndef CENTOS6_9_MINI_PROCESSOR_H
#define CENTOS6_9_MINI_PROCESSOR_H

#include "ProcessorBase.h"

/*
 * @brief response_list 处理 list响应
 * */
class CResponseList : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket* handler, const PBMessage& message)override;
};

/*
 * @brief response_get 处理 开始下载文件的响应
 * */
class CResponseGet : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket* handler, const PBMessage& message)override;
};

/*
 * @brief response_pasv 处理 开启数据传输链路响应
 * */
class CResponsePasv : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket* handler, const PBMessage& message)override;
};

/*
 * @brief response_download 处理 文件传输响应
 * */
class CResponseDownload : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket* handler, const PBMessage& message)override;
};

#endif //CENTOS6_9_MINI_PROCESSOR_H
