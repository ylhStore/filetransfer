//
// Created by ylh on 2021/6/8.
//

#include <sys/stat.h>
#include "CommonModel.h"

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
    ErrMsg("-------------------------------------------------\n");
    ErrMsg("message id[%d]\n", message.msg_case());
    ErrMsg("message [%s]\n", strProto.c_str());
    ErrMsg("-------------------------------------------------\n");
}

void CommonModel::send_message(CHandlerTcpSocket *handler, PBMessage &message)
{
    CPBOutputPacker outputPacker;
    outputPacker.encode_proto_msg(message);
    outputPacker.end();
    handler->append_write(outputPacker.packet_buf(), outputPacker.packet_size());
}

std::string CommonModel::print_command_response_code(const ECommandResponseCode code)
{
    switch (code)
    {
        case E_COMMAND_NOT_EXIST_FILE:
            return "文件不存在";
        case E_COMMAND_ERROR:
            return "命令错误";
        case E_COMMAND_INVALID:
            return "无效命令";
        case E_COMMAND_OK:
            return "命令正确";
        default:
            return "无效code";
    }
}

int CommonModel::decode_port(int hash_port)
{
    return hash_port / 1997;
}

int CommonModel::encode_port(int port)
{
    return port * 1997;
}

void CommonModel::close_handler(CHandlerTcpSocket *handler)
{
    handler->set_need_close(true);
}

void CommonModel::print_rate_of_progress(int64_t num, int64_t tot)
{
    if (tot <= 0 || num < 0)
    {
        return;
    }
    printf("\033[1A");
    printf("%2ld%\n", 100 * num / tot);
}
