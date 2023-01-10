#ifndef CENTOS6_9_MINI_CACHE_H
#define CENTOS6_9_MINI_CACHE_H

class CCache
{
private:
    char * m_mem; // 实际使用的内存
    int m_data_head; // 当前内存的下标
    unsigned int m_size; // 最大内存大小
    unsigned int m_data_len; // 当前数据大小
public:
    CCache(unsigned int size);
    ~CCache()
    {
        delete [] m_mem;
        m_mem = nullptr;
    }
    
    // 返回数据起始位置
    char* data();
    
    // 更新 数据
    void skip(unsigned int length);

    // 返回数据大小
    unsigned int data_len();

    // 添加数据
    void append(const char* src, unsigned int data_len);


    // 重置数据
    void reset();
};


#endif