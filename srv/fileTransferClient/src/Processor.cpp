//
// Created by ylh on 2021/6/8.
//
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "Processor.h"
#include "CommonModel.h"
#include "HandlerClientTransmission.h"
#include "NewReactor.h"

int CResponseList::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    const ResponseList response_list = message.response_list();
    LogMsg("%s\n", CommonModel::print_command_response_code(response_list.res_code()).c_str());

    if (response_list.res_code() == E_COMMAND_OK)
    {
        // list 命令正确 请求开启数据传输
        PBMessage request;
        RequestPASV *request_pasv = request.mutable_request_pasv();
        handler->push_wait_message(request);
    }

    return 0;
}

int CResponseGet::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    const ResponseGet response_get = message.response_get();
    LogMsg("%s\n", CommonModel::print_command_response_code(response_get.res_code()).c_str());
    if (response_get.res_code() == E_COMMAND_OK)
    {
        // get 命令正确 请求开启数据传输
        PBMessage request;
        RequestPASV *request_pasv = request.mutable_request_pasv();
        handler->push_wait_message(request);
    }else{
        printf("%s\n",CommonModel::print_command_response_code(response_get.res_code()).c_str());
    }
    return 0;
}

int CResponsePasv::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    const ResponsePASV response_pasv = message.response_pasv();
    if (response_pasv.res_code() == E_COMMAND_OK)
    {
        CHandlerClientTransmission *transmission_handler = new CHandlerClientTransmission(handler->m_handler_proxy,
                                                                            (CHandlerClient *) handler);
        int port = CommonModel::decode_port(response_pasv.hash_port());
        transmission_handler->init("transmission", response_pasv.ip().c_str(), port,
                                   &CHandlerClientTransmission::transfer_file);
        if (!transmission_handler->connect())
        {
            ErrMsg("CHandlerClientTransmission connect error[%s]\n", strerror(errno));
            delete transmission_handler;
            return 0;
        }

        //控制链路 绑定 数据链路
        CHandlerClient *clientHandler = (CHandlerClient *) handler;
        clientHandler->m_transmission_handler = transmission_handler;

        if (CNewReactor::instance()->register_client(transmission_handler) < 0)
        {
            ErrMsg("CNewReactor::instance()->register_client error\n");
            return 0;
        }
        printf("\n");
    }
    return 0;
}

int CResponseDownload::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    const ResponseDownload response_download = message.response_download();

    CHandlerClientTransmission *transmission = (CHandlerClientTransmission *) handler;
    if (response_download.status() == E_DOWNLOAD_ERROR)
    {
        ErrMsg("download error\n");
        CommonModel::close_handler(handler);
        return 0;
    }

    CHandlerClientTransmission::SFileInfo file_info = transmission->get_file_info();
//    off_t offset = lseek(file_info.m_fd, file_info.m_offset, SEEK_SET);
//    if (-1 == offset)
//    {
//        ErrMsg("lseek error");
//        ErrMsg("fd[%d],offset[%d],file_name[%s]\n", file_info.m_fd, file_info.m_offset, file_info.m_file_name.c_str());
//        CommonModel::close_handler(handler);
//        return 0;
//    }
    int len = response_download.file_info().length();
    int size = write(file_info.m_fd, response_download.file_info().c_str(), len);
    if (len != size)
    {
        ErrMsg("write error");
        ErrMsg("fd[%d],offset[%d],file_name[%s]\n", file_info.m_fd, file_info.m_offset, file_info.m_file_name.c_str());
        CommonModel::close_handler(handler);
        return 0;
    }

    // 更新文件属性信息
    {
        CHandlerClientTransmission::SFileInfo *mutable_file_info = transmission->mutable_file_info();
        mutable_file_info->m_offset += len;
    }

    // 输出进度条
    {
        CommonModel::print_rate_of_progress(transmission->mutable_file_info()->m_offset, response_download.file_size());
    }

    // 数据传输完毕关闭链接
    if (response_download.status() == E_DOWNLOAD_DONE)
    {
        LogMsg("response_download.status() == E_DOWNLOAD_DONE\n");
        CommonModel::close_handler(handler);
        return 0;
    }
    return 0;
}
