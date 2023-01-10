#ifndef _TOOL_SINGLETON_H
#define _TOOL_SINGLETON_H

namespace m_tool
{
    template <typename T>
    class CSingleton
    {
    public:
        T *instance()
        {
            static T data;
            return &data;
        }

        CSingleton() = default;
        ~CSingleton() = default;

    private:
    };
}

#endif //_TOOL_SINGLETON_H