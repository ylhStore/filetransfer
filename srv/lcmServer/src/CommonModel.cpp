//
// Created by ylh on 2021/6/8.
//

#include <sys/stat.h>
#include <sys/socket.h>
#include "CommonModel.h"
#include "Log.h"

bool CommonModel::exist_file(std::string &file)
{
    struct stat st;
    if (-1 == stat(file.c_str(), &st))
    {
        return false;
    } else
    {
        return st.st_mode & S_IFREG;
    }
}

void CommonModel::print_message(const PBMessage &message)
{
    std::string strProto;
    google::protobuf::TextFormat::PrintToString(message, &strProto);
    LogMsg("-------------------------------------------------\n");
    LogMsg("message id[%d]\n", message.msg_case());
    LogMsg("message [%s]\n", strProto.c_str());
    LogMsg("-------------------------------------------------\n");
}

void CommonModel::send_message(CHandlerTcpSocket *handler, PBMessage &message)
{
    CPBOutputPacker outputPacker;
    outputPacker.encode_proto_msg(message);
    outputPacker.end();
    handler->append_write(outputPacker.packet_buf(), outputPacker.packet_size());
}

bool CommonModel::get_empty_port(int range_s, int range_e, int &port, int socket_fd,sockaddr_in& server_addr)
{
    for (int idx_port = range_s; idx_port < range_e; ++idx_port)
    {
        server_addr.sin_port = htons(idx_port);
        if(-1 != bind(socket_fd,(sockaddr*)&server_addr,sizeof(server_addr)))
        {
            port = idx_port;
            return true;
        }
    }
    return false;
}

int CommonModel::decode_port(int hash_port)
{
    return hash_port / 1997;
}

int CommonModel::encode_port(int port)
{
    return port * 1997;
}

void CommonModel::close_handler(CHandlerTcpSocket* handler)
{
    handler->set_need_close(true);
}