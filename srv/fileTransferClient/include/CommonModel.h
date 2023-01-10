//
// Created by ylh on 2021/6/8.
//

#ifndef CENTOS6_9_MINI_COMMONMODEL_H
#define CENTOS6_9_MINI_COMMONMODEL_H

/*
 * 类中全是静态方法 提供processor可能会共用的功能
 * */
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <google/protobuf/text_format.h>
#include "PbMessage.pb.h"
#include "PacketDecoder.h"
#include "TcpSocketHandler.h"
class CommonModel
{
public:
    // 检测文件是否存在
    static bool exist_file(std::string& file);

    // 打印消息内容
    static void print_message(const PBMessage& message);

    // 发送消息
    static void send_message(CHandlerTcpSocket *handler,PBMessage& message);

    // 打印响应值
    static std::string print_command_response_code(const ECommandResponseCode code);

    // 端口解码
    static int decode_port(int hash_port);

    // 端口编码
    static int encode_port(int port);

    // 关闭句柄
    static void close_handler(CHandlerTcpSocket* handler);

    // 输出进度条
    static void print_rate_of_progress(int64_t num,int64_t tot);

};


#endif //CENTOS6_9_MINI_COMMONMODEL_H
