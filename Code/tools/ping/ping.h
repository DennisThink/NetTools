/**
 * @file ping.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief ping功能实现头文件
 * @version 0.1
 * @date 2021-02-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _DENNIS_THINK_PING_H_
#define _DENNIS_THINK_PING_H_
#include <string>
#include <vector>
#include <winsock.h>
#include "icmp_header.h"
namespace dennisthink
{
    struct IcmpRspElem
    {
        std::string m_strIp;//回复的IP
        int m_nBytes;//字节数
        int m_nTime;//时间
        int m_nTTL;//TTL
    };
    class DtPing
    {
    public:
        DtPing();
        ~DtPing();
        int doPing(const std::string strDstIp);
    private:
        bool CreateSocket();
        bool SendIcmpReq();
        bool waitForRecv();
        bool RecvIcmpReply();
        IcmpRspElem RspToResult(const ICMP_Reply& elem);
        bool CloseSocket();
        uint16_t GetCheckSum(const void * pChar,const int len);
        SOCKET m_socket;
        std::string m_strDstIp;
        struct sockaddr_in m_srcAddr;
        struct sockaddr_in m_dstAddr;
        int m_nNum;
        std::vector<IcmpRspElem> m_vec;
    };
}
#endif