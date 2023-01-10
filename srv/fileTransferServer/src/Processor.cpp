//
// Created by ylh on 2021/6/8.
//
#include <sys/types.h>
#include <unistd.h>
#include "Processor.h"
#include "ServerTransfer.h"
#include "HandlerTransmission.h"
#include "NewReactor.h"
#include "CommonModel.h"
#include "Tool.h"
#include "Config.h"

int CRequestList::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    LogMsg("CRequestList::process\n");
    PBMessage response;
    ResponseList *response_list = response.mutable_response_list();
    response_list->set_res_code(E_COMMAND_OK);

    CommonModel::print_message(response);
    CommonModel::send_message(handler, response);
    return 0;
}

int CRequestGet::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    LogMsg("CRequestGet::process\n");

    PBMessage response;
    ResponseGet *response_get = response.mutable_response_get();
    std::string file_name = message.request_get().file_path();

    //查找文件是否存在
    if (CommonModel::exist_file(file_name))
    {
        response_get->set_res_code(E_COMMAND_OK);
        handler->m_transfer_file_name = file_name;
    } else
    {
        response_get->set_res_code(E_COMMAND_NOT_EXIST_FILE);
    }

    CommonModel::print_message(response);
    CommonModel::send_message(handler, response);
    return 0;
}

int CRequestPasv::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    LogMsg("CRequestPasv::process\n");

    // 检测控制链路之前是否申请过文件传输
    if ("" == handler->m_transfer_file_name)
    {
        ErrMsg("CRequestPasv::process error, m_transfer_file_name is empty\n");
        return 0;
    }

    int port = 0;

    { // 获得一个空闲端口
        int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == sock_fd)
        {
            ErrMsg("CRequestPasv::process error, socket error\n");
            return 0;
        }
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(DEFAULT_IP);
        if (!CommonModel::get_empty_port(EMPTY_POERT_RANGE_START, EMPTY_POERT_RANGE_END, port, sock_fd, addr))
        {
            ErrMsg("CRequestPasv::process error, get_empty_port failed\n");
            return 0;
        }
        close(sock_fd);
    }

    // 开启一个数据传输监听
    CServerTransfer *transfer = new CServerTransfer(handler->m_handler_proxy, DEFAULT_ACCEPT_CNT, DEFAULT_BACK_LOG);
    transfer->init("transfer" + m_tool::unique_string_id(), DEFAULT_IP, port);
    if (!CNewReactor::instance()->bind_server(transfer))
    {
        ErrMsg("CRequestPasv::process CNewReactor::instance()->bind_server error[%s]\n", strerror(errno));
        return 0;
    }
    transfer->m_transfer_file_name = handler->m_transfer_file_name;

    // 控制链路 和 传输链路相互绑定
    transfer->m_control_handler = handler;
    handler->m_transfer_handler = transfer;

    PBMessage response;
    ResponsePASV *response_pasv = response.mutable_response_pasv();
    response_pasv->set_res_code(E_COMMAND_OK);
    response_pasv->set_ip(TRANSFER_IP);
    response_pasv->set_hash_port(CommonModel::encode_port(port));

    CommonModel::print_message(response);
    CommonModel::send_message(handler, response);
    return 0;
}

int CRequestDownload::process(CHandlerTcpSocket *handler, const PBMessage &message)
{
    LogMsg("CRequestDownload::process\n");

    CHandlerTransmission *transmission = (CHandlerTransmission *) handler;
    CHandlerTransmission::SFileInfo *file_info = transmission->mutable_file_info();

    PBMessage response;
    ResponseDownload *response_download = response.mutable_response_download();

    // 更新传输文件信息
    if (file_info->m_fd == -1)
    {
        // 没有打开过文件 打开文件
        file_info->m_fd = open(file_info->m_file_name.c_str(), O_RDONLY | O_EXCL);

        // 打开文件失败
        if (-1 == file_info->m_fd)
        {
            ErrMsg("CRequestDownload::process error, open file[%s] error[%s]\n", file_info->m_file_name.c_str(),
                   strerror(errno));
            response_download->set_status(E_DOWNLOAD_ERROR);
            CommonModel::print_message(response);
            CommonModel::send_message(handler, response);
            return 0;
        }

        // 获取文件信息失败
        if (-1 == stat(file_info->m_file_name.c_str(), &file_info->m_file_st))
        {
            ErrMsg("CRequestDownload::process error,get file[%s] stat error[%s]\n", file_info->m_file_name.c_str(),
                   strerror(errno));
            response_download->set_status(E_DOWNLOAD_ERROR);
            CommonModel::print_message(response);
            CommonModel::send_message(handler, response);
            return 0;
        }
    }

    // 响应中填充文件总大小
    {
        response_download->set_file_size(file_info->m_file_st.st_size);
    }

    const RequestDownload request_download = message.request_download();
    int offset = request_download.offset();

    // 检测请求的偏移值是否有效
    if (offset < 0 || offset > file_info->m_file_st.st_size)
    {
        response_download->set_status(E_DOWNLOAD_DONE);
        response_download->set_file_info("");
        CommonModel::print_message(response);
        CommonModel::send_message(handler, response);
        return 0;
    }

    // 设置文件读写位置
    if (-1 == lseek(file_info->m_fd, offset, SEEK_SET))
    {
        ErrMsg("CRequestDownload::process error, lseek file[%s] error[%s]\n", file_info->m_file_name.c_str(),
               strerror(errno));
        return 0;
    }

    // 填充文件数据
    const int BUFFER_SIZE = request_download.buffer_size();
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int len = read(file_info->m_fd, buffer, BUFFER_SIZE);
    if (-1 == len)
    {
        ErrMsg("CRequestDownload::process error, read file[%s] error[%s]\n", file_info->m_file_name.c_str(),
               strerror(errno));
        return 0;
    }
    response_download->mutable_file_info()->append(buffer,len);

    // 设置文件内容大小
    response_download->set_file_info_size(response_download->file_info().size());

    // 设置文件传输状态
    if (offset + len >= file_info->m_file_st.st_size)
    {
        response_download->set_status(E_DOWNLOAD_DONE);
        // 设置需要关闭连接
        CommonModel::close_handler(handler);
    } else
    {
        response_download->set_status(E_DOWNLOAD_ING);
    }

    // 发送消息
    CommonModel::send_message(handler, response);

    return 0;
}