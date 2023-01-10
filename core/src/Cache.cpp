#include <string.h>
#include "Cache.h"

CCache::CCache(unsigned int size)
{
    m_data_len = 0;
    m_size = size;
    m_data_head = 0;
    m_mem = new char[m_size];
    memset(m_mem, 0, sizeof(char) * m_size);
}

char *CCache::data()
{
    return m_mem + m_data_head;
}

void CCache::skip(unsigned int length)
{
    if (length <= m_data_len)
    {
        m_data_head += length;
        m_data_len -= length;
    }
}

unsigned int CCache::data_len()
{
    return m_data_len;
}

void CCache::append(const char *data, unsigned int data_len)
{
    // 可以直接添加数据
    if (data_len + m_data_head + m_data_len <= m_size)
    {
        memcpy(m_mem + m_data_head + m_data_len, data, data_len);
    }
    else
    {
        // 需要把之前的数据向前移动后 再添加新数据
        memmove(m_mem, m_mem + m_data_head, m_data_len);
        m_data_head = 0;
        memcpy(m_mem + m_data_head + m_data_len, data, data_len);
    }
    m_data_len += data_len;
}

void CCache::reset()
{
    m_data_len = 0;
    m_data_head = 0;
}
