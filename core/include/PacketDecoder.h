//
// Created by ylh on 2021/6/3.
//

#ifndef CENTOS6_9_MINI_PACKET_DECODER_H
#define CENTOS6_9_MINI_PACKET_DECODER_H

#include <arpa/inet.h>
#include <string>
#include <cstring>
#include "PbMessage.pb.h"
#include "Log.h"
#include "Global.h"

/*
 * @ brief 定义包头内容
 * */
struct SPacketHeader
{
    unsigned int pack_len;       // 数据包长度
    unsigned char check_code[4]; // 分隔符 保证pack_len的有效性 默认分隔符是“test”
    SPacketHeader()
    {
        pack_len = 0;
        memcpy(check_code, "test", sizeof(check_code));
    }

    SPacketHeader(unsigned int source_len)
    {
        pack_len = source_len + sizeof(SPacketHeader);
        memcpy(check_code, "test", sizeof(check_code));
    }
};

/*
 * @ brief 数据包处理基类，封装基本读写操作
 * */
class CPacketBase
{
protected:
    int m_packet_size;                  // 数据包 大小
    int m_buf_pos;                      // 当前数据包操作额下标
    char m_str_buf[MAX_TCP_BUFFER_LEN]; // 存放数据的数据
public:
    const int PACKET_HEADER_SIZE;
    const int PACKER_BUFFER_SIZE;

    CPacketBase(
        const int _packet_header_size = sizeof(SPacketHeader),
        const int _packet_buffer_size = MAX_TCP_BUFFER_LEN)
        : PACKET_HEADER_SIZE(_packet_header_size),
          PACKER_BUFFER_SIZE(_packet_buffer_size)
    {
    }
    virtual ~CPacketBase()
    {
    }
    /*
     * @ brief 获得数据包数据
     * @ return 数据包数据
     * */
    char *packet_buf()
    {
        return m_str_buf;
    }

    /*
     * @ brief 获得数据长度
     * @ return 数据包长度
     * */
    int packet_size() const
    {
        return m_packet_size;
    }

    /*
     * @ brief 重置数据包信息
     * */
    void _reset()
    {
        memset(m_str_buf, 0, MAX_TCP_BUFFER_LEN);
        m_buf_pos = PACKET_HEADER_SIZE;
        m_packet_size = PACKET_HEADER_SIZE;
    }

    /*
     * @ brief 数据包复制数据
     * @ param p_src 复制的数据源
     * @ param n_len 数据源长度
     * @ return 是否复制成功
     * */
    bool _copy(const void *p_src, int n_len)
    {
        if (n_len > PACKER_BUFFER_SIZE)
        {
            return false;
        }
        _reset();
        memcpy(m_str_buf, p_src, n_len);
        m_packet_size = n_len;
        return true;
    }

    /*
     * @ brief 从数据包中读数据
     * @ param p_out 要从数据包读取数据的数据目的地
     * @ param n_len 要读取的数据长度
     * @ return 是否读取成功
     * */
    bool _read(char *p_out, int n_len)
    {
        if ((n_len + m_buf_pos) > m_packet_size || (n_len + m_buf_pos) > PACKER_BUFFER_SIZE)
        {
            return false;
        }
        memcpy(p_out, m_str_buf + m_buf_pos, n_len);
        m_buf_pos += n_len;
        return true;
    }

    /*
     * @ brief 撤销从数据包中读数据操作
     * @ param n_len 撤销的读取长度
     * */
    void _read_undo(int n_len)
    {
        m_buf_pos -= n_len;
    }

    /*
     * @ brief 从数据包中读数据
     * @ param n_len 要读取的数据长度
     * @ return 要读取的数据在数据包中的地址
     * */
    char *_read_point(int n_len)
    {
        if ((n_len + m_buf_pos) > m_packet_size)
        {
            return nullptr;
        }
        char *p = &m_str_buf[m_buf_pos];
        m_buf_pos += n_len;
        return p;
    }

    /*
     * @ brief 向数据包中写数据
     * @ param p_in 向数据包中写数据的数据源
     * @ param n_len 要写的数据长度
     * @ return 是否写入成功
     * */
    bool _write(const char *p_in, int n_len)
    {
        if ((m_packet_size < 0) || ((n_len + m_packet_size) > PACKER_BUFFER_SIZE))
        {
            return false;
        }
        memcpy(m_str_buf + m_packet_size, p_in, n_len);
        m_packet_size += n_len;
        return true;
    }

    /*
     * @ brief 向数据包中写数据0
     * @ return 是否写入成功
     * */
    bool _write_zero()
    {
        if ((m_packet_size + 1) > PACKER_BUFFER_SIZE)
        {
            return false;
        }
        memset(m_str_buf + m_packet_size, '\0', sizeof(char));
        m_packet_size++;
        return true;
    }

    /*
     * @ brief 向数据包中写包头信息
     * @ param p_in 向数据包中写数据的数据源
     * @ param n_len 要写的数据长度
     * @ param n_pos 要写入的位置
     * */
    void _write_header(char *p_in, int n_len, int n_pos)
    {
        if (n_pos > 0 || (n_pos + n_len) < PACKET_HEADER_SIZE)
        {
            memcpy(m_str_buf + n_pos, p_in, n_len);
        }
    }

    /*
     * @ brief 向数据包写入完数据之后填充包头信息
     * */
    void _end()
    {
        char check_code[4] = {'t', 'e', 's', 't'};
        _write_header((char *)&check_code, sizeof(check_code), sizeof(int));
        int len = htonl(m_packet_size);
        _write_header((char *)&len, sizeof(len), 0);
    }
};

/*
 * @ brief 读数据包操作类 封装读int,short等数据类型的操作
 * */
class CInputPacker : public CPacketBase
{
public:
    typedef CPacketBase base;

    CInputPacker(const int _packet_header_size = sizeof(SPacketHeader),
                 const int _packet_buffer_size = MAX_TCP_BUFFER_LEN) : CPacketBase(_packet_header_size, _packet_buffer_size)
    {
    }

    CInputPacker(const char *data, int len, const int _packet_header_size = sizeof(SPacketHeader),
                 const int _packet_buffer_size = MAX_TCP_BUFFER_LEN) : CPacketBase(_packet_header_size, _packet_buffer_size)
    {
        _copy(data, len);
    };

    virtual ~CInputPacker()
    {
    }

    int read_int()
    {
        int value = 0;
        _read((char *)&value, sizeof(value));
        return ntohl(value);
    }

    uint32_t read_uint32()
    {
        uint32_t value = 0;
        _read((char *)&value, sizeof(value));
        return ntohl(value);
    }

    unsigned long read_unsigned_long()
    {
        unsigned long value = 0;
        _read((char *)&value, sizeof(value));
        return ntohl(value);
    }

    short read_short()
    {
        short value = 0;
        _read((char *)&value, sizeof(short));
        return ntohs(value);
    }

    unsigned char read_byte()
    {
        unsigned char value = 0;
        _read((char *)&value, sizeof(unsigned char));
        return value;
    }

    bool read_string(char *p_out_string, int n_max_len)
    {
        int n_len = read_int();
        if (n_len > n_max_len)
        {
            _read_undo(sizeof(n_len));
            return false;
        }
        return _read(p_out_string, n_len);
    }

    char *read_char()
    {
        int n_len = read_int();
        return _read_point(n_len);
    }

    std::string read_string()
    {
        char *p = read_char();
        return nullptr == p ? "" : p;
    }

    int read_binary(char *p_buf, int n_max_len)
    {
        int n_len = read_int();
        if (n_len < 0)
        {
            return -1;
        }
        if (n_len > n_max_len)
        {
            _read_undo(sizeof(n_len));
            return -2;
        }
        if (_read(p_buf, n_len))
        {
            return n_len;
        }
        return 0;
    }

    void reset()
    {
        _reset();
    }

    bool copy(const void *p_in_buf, int n_len)
    {
        return _copy(p_in_buf, n_len);
    }

    bool write_body(const char *p_in, int n_len)
    {
        int value = htonl(n_len);
        _write((char *)&value, sizeof(int));
        return _write(p_in, n_len);
    }

    void end()
    {
        return _end();
    }
};

/*
 * @ brief 写数据包操作类 封装写int,short等数据类型的操作
 * */
class COutputPacket : public CPacketBase
{
public:
    COutputPacket(const int _packet_header_size = sizeof(SPacketHeader),
                  const int _packet_buffer_size = MAX_TCP_BUFFER_LEN) : CPacketBase(_packet_header_size, _packet_buffer_size)
    {
        _reset();
    }

    virtual ~COutputPacket() {}

    typedef CPacketBase base;

    bool write_int(int nValue)
    {
        int value = htonl(nValue);
        return base::_write((char *)&value, sizeof(int));
    }

    bool write_uint32(uint32_t nValue)
    {
        uint32_t value = htonl(nValue);
        return base::_write((char *)&value, sizeof(int));
    }

    bool write_unsigned_long(unsigned long nValue)
    {
        unsigned long value = htonl(nValue);
        return base::_write((char *)&value, sizeof(unsigned long));
    }

    bool write_byte(unsigned char nValue)
    {
        return base::_write((char *)&nValue, sizeof(unsigned char));
    }

    bool write_short(short nValue)
    {
        short value = htons(nValue);
        return base::_write((char *)&value, sizeof(short));
    }

    bool write_string(const char *pString)
    {
        int nLen = (int)strlen(pString);
        write_int(nLen + 1);
        return base::_write(pString, nLen) && base::_write_zero();
    }

    bool write_string(const std::string &strDate)
    {
        int nLen = (int)strDate.size();
        write_int(nLen + 1);
        return base::_write(strDate.c_str(), nLen) && base::_write_zero();
    }

    bool write_binary(const char *pBuf, int nLen)
    {
        write_int(nLen);
        return base::_write(pBuf, nLen);
    }

    bool copy(const void *pInBuf, int nLen)
    {
        return base::_copy(pInBuf, nLen);
    }

    void end()
    {
        return base::_end();
    }
};

/*
 * @ brief 读pb数据操作类 解码pb数据
 * */
class CPBInputPacker : public CInputPacker
{
private:
public:
    CPBInputPacker(const char *buf, int size) : CInputPacker(buf, size)
    {
        // CInputPacker(buf, size);
    }

    CPBInputPacker(){};
    virtual ~CPBInputPacker() {}

public:
    bool decode_proto_msg(PBMessage &msg)
    {
        char buff[MAX_TCP_BUFFER_LEN];
        memset(buff, 0, sizeof(buff));
        int ret = read_binary(buff, MAX_TCP_BUFFER_LEN);
        if (ret <= 0)
        {
            ErrMsg("read_binary failed, ret[%d]\n", ret);
            return false;
        }
        if (!msg.ParseFromArray(buff, ret))
        {
            ErrMsg("msg.ParseFromArray failed");
            return false;
        }
        return true;
    }
};

/*
 * @ brief 写pb数据操作类 编码pb数据
 * */
class CPBOutputPacker : public COutputPacket
{
private:
public:
    CPBOutputPacker()
    {
        _reset();
    };

    virtual ~CPBOutputPacker() {}

public:
    bool encode_proto_msg(const PBMessage &msg)
    {
        char buff[MAX_TCP_BUFFER_LEN];
        memset(buff, 0, sizeof(buff));
        int size = msg.ByteSizeLong();
        if (size > MAX_TCP_BUFFER_LEN)
        {
            ErrMsg("invalid byte size[%d]\n", size);
            return false;
        }
        msg.SerializeToArray(buff, size);
        write_binary(buff, size);
        return true;
    }
};

/*
 * @ brief 数据包包头处理基类，封装检查数据包包头操作
 * */
class CPacket
{
public:
    CPacket();

    virtual ~CPacket();

    virtual int get_head_len();

    virtual bool check_head(const char *data, int len);

    virtual int get_packet_len(const char *data, int len);
};

/*
 * @ brief 数据包解码类 检测数据包包头信息
 * */
class CPacketDecoder : public CPacket
{
private:
    typedef CPacket base;

public:
    CPacketDecoder();

    virtual ~CPacketDecoder();

    virtual int get_head_len() { return base::get_head_len(); }

    virtual bool check_head(const char *data, int len) { return base::check_head(data, len); }

    virtual int get_packet_len(const char *data, int len) { return base::get_packet_len(data, len); }

    virtual int unpack(const char *data, int len);
};

/*
 * @ brief 数据包编码类 （已弃用）
 * */
class CPacketEncoder : public CPacket
{
private:
    typedef CPacket base;

public:
    CPacketEncoder();

    virtual ~CPacketEncoder();

    virtual int get_head_len() { return base::get_head_len(); }

    virtual bool check_head(const char *data, int len) { return base::check_head(data, len); }

    virtual int get_packet_len(const char *data, int len) { return base::get_packet_len(data, len); }

    virtual int pack(const char *data, int len, std::string &dest);
};

#endif //CENTOS6_9_MINI_PACKET_DECODER_H
