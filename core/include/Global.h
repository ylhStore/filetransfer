#ifndef CENTOS6_9_MINI_GLOBAL_H
#define CENTOS6_9_MINI_GLOBAL_H

const unsigned int MAX_PRE_LEN = 64;

const unsigned int MAX_MSG_LEN = 1024*16;

const unsigned int MAX_RECV_LEN = MAX_MSG_LEN + MAX_PRE_LEN;

const unsigned int MAX_TCP_BUFFER_LEN = MAX_MSG_LEN + MAX_PRE_LEN;

#endif