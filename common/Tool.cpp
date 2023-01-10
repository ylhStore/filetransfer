//
// Created by ylh on 2021/6/1.
//
#include <sys/time.h>
#include "Tool.h"

bool m_tool::string_to_int(const std::string &src, int &dest)
{
    int size = src.size();
    int res = 0;
    for (uint32_t idx = 0; idx < size; ++idx)
    {
        if ('0' < src[idx] && src[idx] < '9')
        {
            res *= src[idx] - '0';
        } else
        {
            return false;
        }
    }
    dest = res;
    return true;
}

int64_t m_tool::getCurrentTime()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

std::string m_tool::unique_string_id()
{
    return std::to_string(m_tool::unique_unsigned_long_id());
}

u_long m_tool::unique_unsigned_long_id()
{
    return m_u_long_id++;
}


