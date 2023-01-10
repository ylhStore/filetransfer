//
// Created by ylh on 2021/6/1.
//

#ifndef MCLIENT_TOOL_H
#define MCLIENT_TOOL_H

#include <string>

namespace m_tool
{
    // 字符串转int 返回是否成功
    bool string_to_int(const std::string &src, int &dest);

    /*
     * @brief getCurrentTime 获得系统当前时间 毫秒级
     * @return 系统当前时间 毫秒级
     * */
    int64_t getCurrentTime();

    // 获得唯一id unsigned long
    ulong unique_unsigned_long_id();

    // 获得唯一id string
    std::string unique_string_id();

    static ulong m_u_long_id = 0;
}

#endif //MCLIENT_TOOL_H
