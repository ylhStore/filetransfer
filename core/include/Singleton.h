//
// Created by ylh on 2021/6/1.
//

#ifndef CENTOS6_9_MINI_SINGLETON_H
#define CENTOS6_9_MINI_SINGLETON_H
/*
 * @ brief 单例类模板，需要是单例的类继承该类即可
 * */
template<typename ObjectType>
class CSingleton
{
public:
    static ObjectType *instance()
    {
        return &reference();
    }
    static ObjectType& reference()
    {
        static ObjectType obj;
        return obj;
    }

protected:
    CSingleton()
    {}

    CSingleton(CSingleton const &)
    {}
};

#endif //CENTOS6_9_MINI_SINGLETON_H
