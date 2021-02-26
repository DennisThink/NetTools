#include "ping.h"
#include <iostream>
#include <winsock.h>
#include "icmp_header.h"
#pragma comment(lib, "ws2_32.lib")
namespace dennisthink
{

    static volatile int keepping = 1;

    //https ://blog.csdn.net/chenpidaxia/article/details/77803388

    
    //判断参数是否为空，空的话返回一个NULL指针，否侧提取参数存入到param中，同时返回
    char *isParamEmpty(char *buffer, char *param);
    //捕获终止信号函数
    void get_ctrl_stop(int signal);

    char *isParamEmpty(char *buffer, char *param)
    {
        char *temp = NULL;
        temp = buffer;
        while (*temp != '\0')
        {
            if (*temp == ' ')
            {
                *temp = '\0';
                param = ++temp;
            }
            temp++;
        }
        return param;
    }
    bool DtPing::CreateSocket()
    {

        //生成一个套接字
        //TCP/IP协议族,RAW模式，ICMP协议
        //RAW创建的是一个原始套接字，最低可以访问到数据链路层的数据，也就是说在网络层的IP头的数据也可以拿到了。
        m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

        //设置目标IP地址
        m_dstAddr.sin_addr.S_un.S_addr = inet_addr(m_strDstIp.c_str());
        //端口
        m_dstAddr.sin_port = htons(0);
        //协议族
        m_dstAddr.sin_family = AF_INET;
        return true;
    }
    bool DtPing::SendIcmpReq()
    {
        //创建ICMP请求回显报文
        //设置回显请求
        ICMP_Req myIcmp; //ICMP请求报文
        myIcmp.icmphead.type = 8;
        myIcmp.icmphead.code = 0;
        //设置初始检验和为0
        myIcmp.icmphead.checkSum = 0;
        //获得一个进程标识
        myIcmp.icmphead.ident = (uint16_t)GetCurrentProcessId();
        //设置当前序号为0
        myIcmp.icmphead.seqNum = ++m_nNum;
        //保存发送时间
        myIcmp.timeStamp = GetTickCount();
        //计算并且保存校验和
        myIcmp.icmphead.checkSum = GetCheckSum((void *)&myIcmp, static_cast<int>(sizeof(ICMP_Req)));
        //发送报文
        int Ret = sendto(m_socket, (char *)&myIcmp, sizeof(ICMP_Req), 0, (sockaddr *)&m_dstAddr, sizeof(sockaddr_in));

        if (Ret == SOCKET_ERROR)
        {
            std::cerr << "socket send error:" << WSAGetLastError() << std::endl;
            return false;
        }
        return true;
    }
    bool DtPing::waitForRecv()
    {
        //5S 等待套接字是否由数据
        timeval timeOut;
        fd_set readfd;
        readfd.fd_count = 1;
        readfd.fd_array[0] = m_socket;
        timeOut.tv_sec = 5;
        timeOut.tv_usec = 0;
        int nRet = select(1, &readfd, NULL, NULL, &timeOut);
        return (nRet != 0 && nRet != SOCKET_ERROR);
    }
    bool DtPing::RecvIcmpReply()
    {
        ICMP_Reply icmpReply; //接收应答报文
        int addrLen = sizeof(sockaddr_in);
        //接收应答
        int Ret = recvfrom(m_socket, (char *)&icmpReply, sizeof(ICMP_Reply), 0, (sockaddr *)&m_srcAddr, &addrLen);
        if (Ret == SOCKET_ERROR)
        {
            std::cerr << "socket recv error:" << WSAGetLastError() << std::endl;
            return false;
        }
        //读取校验并重新计算对比
        uint16_t checkSum = icmpReply.icmpanswer.icmphead.checkSum;
        //因为发出去的时候计算的校验和是0
        icmpReply.icmpanswer.icmphead.checkSum = 0;
        //重新计算
        IcmpRspElem result = RspToResult(icmpReply);
        std::cout << "IP: " << result.m_strIp << " Bytes: " << result.m_nBytes << " TTL: " << result.m_nTTL << std::endl;
        return true;
    }
    IcmpRspElem DtPing::RspToResult(const ICMP_Reply &elem)
    {
        IcmpRspElem result;
        result.m_strIp = inet_ntoa(m_srcAddr.sin_addr);
        result.m_nBytes = elem.iphead.totalLength;
        result.m_nTTL = -(elem.iphead.timetoLive);
        return result;
    }
    uint16_t DtPing::GetCheckSum(const void *pChar, const int len)
    {
        uint32_t checkSum = 0;
        uint16_t *word = (uint16_t *)pChar;
        uint32_t size = len;
        while (size > 1) //用32位变量来存是因为要存储16位数相加可能发生的溢出情况，将溢出的最高位最后加到16位的最低位上
        {
            checkSum += *word++;
            size -= 2;
        }
        if (size == 1)
        {
            checkSum += *(uint8_t *)word;
        }
        //二进制反码求和运算，先取反在相加和先相加在取反的结果是一样的，所以先全部相加在取反
        //计算加上溢出后的结果
        while (checkSum >> 16)
        {
            checkSum = (checkSum >> 16) + (checkSum & 0xffff);
        }
        //取反
        return (~checkSum);
    }

    bool DtPing::CloseSocket()
    {
        closesocket(m_socket);
        m_socket = -1;
        return true;
    }
    uint16_t getCheckSum(void *protocol, char *type)
    {
        uint32_t checkSum = 0;
        uint16_t *word = (uint16_t *)protocol;
        uint32_t size = 0;
        if (type == "ICMP")
        { //计算有多少个字节
            size = (sizeof(ICMP_Req));
        }
        while (size > 1) //用32位变量来存是因为要存储16位数相加可能发生的溢出情况，将溢出的最高位最后加到16位的最低位上
        {
            checkSum += *word++;
            size -= 2;
        }
        if (size == 1)
        {
            checkSum += *(uint8_t *)word;
        }
        //二进制反码求和运算，先取反在相加和先相加在取反的结果是一样的，所以先全部相加在取反
        //计算加上溢出后的结果
        while (checkSum >> 16)
        {
            checkSum = (checkSum >> 16) + (checkSum & 0xffff);
        }
        //取反
        return (~checkSum);
    }

    //捕获终止信号函数,专门处理无限ping时的操作
    void get_ctrl_stop(int signal)
    {
        /*if (signal == SIGINT)
    {
        keepping = 0;
    }*/
    }
    DtPing::DtPing()
    {
        m_nNum = 0;
    }

    DtPing::~DtPing()
    {
    }

    int DtPing::doPing(const std::string strDstIp)
    {
        m_strDstIp = strDstIp;
        WSADATA wsaData;
        int nRet;
        if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
        {
            printf("WSAStartup failed\n");
            return 0;
        }
        if (!CreateSocket())
        {
            goto ERR;
        }
        for (int i = 0; i < 4; i++)
        {
            if (SendIcmpReq())
            {
                if (waitForRecv())
                {
                    if (RecvIcmpReply())
                    {
                    }
                }
            }
        }
    //ping(strDstIp.c_str(), "-t");
    ERR:
        CloseSocket();
        WSACleanup();
        return 0;
    }
}