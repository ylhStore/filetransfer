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
#include <netinet/in.h>
#include <string>
#include <google/protobuf/text_format.h>
#include "PbMessage.pb.h"
#include "PacketDecoder.h"
#include "TcpSocketHandler.h"

class CommonModel
{
public:
    /*
     * @brief 检测是否存在文件 file
     * */
    static bool exist_file(std::string &file);

    static void print_message(const PBMessage &message);

    static void send_message(CHandlerTcpSocket *handler, PBMessage &message);

    static bool get_empty_port(int range_s, int range_e, int &port, int socket_fd, sockaddr_in &server_addr);

    static int decode_port(int hash_port);

    static int encode_port(int port);

    static void close_handler(CHandlerTcpSocket* handler);
};


#endif //CENTOS6_9_MINI_COMMONMODEL_H
