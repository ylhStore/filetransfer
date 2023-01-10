//
// Created by ylh on 2021/6/8.
//

#ifndef CENTOS6_9_MINI_PROCESSOR_H
#define CENTOS6_9_MINI_PROCESSOR_H

#include "ProcessorBase.h"

/*
 * @brief request_list 处理 list请求
 * */
class CRequestList : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket *handler, const PBMessage &message) override;
};

/*
 * @brief request_get 处理 get请求 请求开始下载文件
 * */
class CRequestGet : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket *handler, const PBMessage &message) override;
};

/*
 * @brief request_pasv 处理 pasv请求 请求开启数据传输
 * */
class CRequestPasv : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket *handler, const PBMessage &message) override;
};

/*
 * @brief request_download 处理 download请求 请求下载文件
 * */
class CRequestDownload : public CProcessorBase
{
public:
    int process(CHandlerTcpSocket *handler, const PBMessage &message) override;
};

#endif //CENTOS6_9_MINI_PROCESSOR_H
