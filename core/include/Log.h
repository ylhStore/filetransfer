#ifndef CENTOS6_9_MINI_LOG_H
#define CENTOS6_9_MINI_LOG_H

// 定义 log 输出
#ifdef _DEBUG_LOG_MSG

#define LogMsg(format,...) \
do{                        \
printf(format,##__VA_ARGS__);\
}                          \
while(false)

#else
#define LogMsg(format,...) \
{}

#endif

// 定义 error 输出
#ifdef _DEBUG_ERR_MSG

#define ErrMsg(format,...) \
do{                        \
printf(format,##__VA_ARGS__);\
}                          \
while(false)

#else
#define ErrMsg(format,...) \
{}

#endif

#endif //CENTOS6_9_MINI_LOG_H