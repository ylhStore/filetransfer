#ifndef CENTOS6_9_MINI_STATICLINK_H
#define CENTOS6_9_MINI_STATICLINK_H

template <typename T>
struct SNode
{
    T *m_data_ptr;
    SNode<T> *next;
    SNode()
    {
        m_data_ptr = nullptr;
        next = nullptr;
    }
    SNode(T *data)
    {
        m_data_ptr = data;
        next = nullptr;
    }
};

template <typename T, template <typename E> class Node = SNode>
class CStaticLink
{
private:
    unsigned int m_size;   // 静态链表大小
    Node<T> *m_link;       // 静态链表
    Node<T> *m_empty_node; // 空闲结点
public:
    CStaticLink();
    ~CStaticLink();
    /*
     * @ brief 初始化链表
     * @ return 是否初始化成功
    */
    bool init(unsigned int size);
    /*
     * @ brief 删除指定结点数据
    */
    void free_node(Node<T> *node);

    /*
     * @ brief 申请结点
     * @ return 返回结点存放的位置
    */
    Node<T> *alloc_node();

    /*
     * @ brief 判断是否是静态链表中的空链表头结点
     * @ return 是否是静态链表中的空链表头结点
    */
    bool is_null_node(Node<T> *node);
};

template <typename T, template <typename E> class Node>
CStaticLink<T, Node>::CStaticLink() : m_size(0),
                                      m_link(nullptr),
                                      m_empty_node(nullptr)
{
}

template <typename T, template <typename E> class Node>
CStaticLink<T, Node>::~CStaticLink()
{
    if (nullptr != m_link)
    {
        delete[] m_link;
        m_link = nullptr;
    }
    m_empty_node = nullptr;
}

template <typename T, template <typename E> class Node>
bool CStaticLink<T, Node>::init(unsigned int size)
{
    m_size = size;
    m_link = new Node<T>[m_size];

    if (nullptr == m_link)
    {
        return false;
    }

    for (int idx = m_size - 2; idx >= 0; --idx)
    {
        m_link[idx].next = &m_link[idx + 1];
    }
    m_empty_node = m_link;
    return true;
}

template <typename T, template <typename E> class Node>
void CStaticLink<T, Node>::free_node(Node<T> *node)
{
    node->next = m_empty_node;
    node->m_data_ptr = nullptr;
    m_empty_node = node; 
}

template <typename T, template <typename E> class Node>
Node<T>* CStaticLink<T,Node>::alloc_node()
{
    if(nullptr == m_empty_node)
    {
        return nullptr;
    }
    Node<T>* res = m_empty_node;
    m_empty_node = m_empty_node->next;

    return res;
}

template <typename T, template <typename E> class Node>
bool CStaticLink<T,Node>::is_null_node(Node<T> *node)
{
    return m_empty_node == node;
}

#endif //CENTOS6_9_MINI_STATICLINK_H