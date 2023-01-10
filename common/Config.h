//
// Created by ylh on 2021/6/4.
//

#ifndef CENTOS6_9_MINI_COMMON_H
#define CENTOS6_9_MINI_COMMON_H

const unsigned int EMPTY_POERT_RANGE_START = 10000;
// const unsigned int EMPTY_POERT_RANGE_START = 5000;

const unsigned int EMPTY_POERT_RANGE_END = 11000;

#define DEFAULT_ACCEPT_CNT (256)
#define DEFAULT_BACK_LOG (256)
#define DEFAULT_PORT (EMPTY_POERT_RANGE_START)
// #define DEFAULT_IP ("172.24.196.203")
// #define TRANSFER_IP ("47.108.156.47")
// // 阿里云服务器 有nat映射 外网链接的ip地址跟内网链接的ip地址不同

#define DEFAULT_IP ("127.0.0.1")
#define TRANSFER_IP ("127.0.0.1")

#endif //CENTOS6_9_MINI_COMMON_H
