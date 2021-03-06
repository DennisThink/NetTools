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
        if(INVALID_SOCKET == m_socket)
        {
            return false;
        }
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
        char chStart = 'a';
        for (int i = 0; i < 32; i++)
        {
            myIcmp.data[i] = chStart + i % 26;
        }
        myIcmp.icmphead.checkSum = GetCheckSum((void *)&myIcmp, static_cast<int>(sizeof(ICMP_Req)));

        //发送报文
        int Ret = sendto(m_socket, (char *)&myIcmp, sizeof(ICMP_Req), 0, (sockaddr *)&m_dstAddr, sizeof(sockaddr_in));

        if (Ret == SOCKET_ERROR)
        {
            std::cerr << "socket send error:" << WSAGetLastError() << std::endl;
            return false;
        }
        m_nSendCount++;
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
        m_vec.push_back(result);
        std::cout << "Response From " << result.m_strIp << " Bytes=" << result.m_nBytes << " time="<< result.m_nTime <<"ms TTL=" << result.m_nTTL << std::endl;
        m_nRecvCount++;
        return true;
    }

    IcmpRspElem DtPing::RspToResult(const ICMP_Reply &elem)
    {
        IcmpRspElem result;
        result.m_strIp = inet_ntoa(m_srcAddr.sin_addr);
        result.m_nBytes = ntohs(elem.iphead.totalLength)-sizeof(IPhead)-sizeof(ICMP_head)-4;
        result.m_nTTL = elem.iphead.timetoLive;
        result.m_nTime = GetTickCount()-elem.icmpanswer.timeStamp;
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
            checkSum += *(uint8_t *)word*256;
        }
        //uint32_t temp = (checkSum >> 16) + (checkSum & 0xffff);
        //二进制反码求和运算，先取反在相加和先相加在取反的结果是一样的，所以先全部相加在取反
        //计算加上溢出后的结果
        checkSum = (checkSum >> 16) + (checkSum & 0xffff);
        /*if(checkSum > 0xffff)
        {
            checkSum = (checkSum >> 16) + (checkSum & 0xffff);
        }*/


        uint16_t result = checkSum;

        result = ~result;
        //取反
        return (result);
    }

    bool DtPing::CloseSocket()
    {
        closesocket(m_socket);
        m_socket = -1;
        return true;
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
            std::cerr << "Create Socket failed Errno: "<<GetLastError()<<std::endl;
            goto ERR;
        }
        ClearStatistic();
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
            Sleep(1000);
        }
        CalcStatisticAndPrint();
    //ping(strDstIp.c_str(), "-t");
    ERR:
        CloseSocket();
        WSACleanup();
        return 0;
    
    }

    void DtPing::ClearStatistic()
    {
        m_nSendCount=0;
        m_nRecvCount=0;
        m_vec.clear();
    }

    void DtPing::CalcStatisticAndPrint()
    {
        int nMin = 0;
        int nMax = 0;
        int nAverage = 0;
        int nSum = 0;
        if(!m_vec.empty())
        {
            nMin = nMax=m_vec[0].m_nTime;
            for(const auto& item:m_vec)
            {
                if(nMax < item.m_nTime )
                {
                    nMax = item.m_nTime;
                }

                if(nMin > item.m_nTime)
                {
                    nMin = item.m_nTime;
                }
                nSum += item.m_nTime;
            }
            nAverage = nSum / m_vec.size();
        }
        std::cout<<m_strDstIp<<" 的Ping统计信息:"<<std::endl;
        std::cout<<"   数据包: 已发送 = "<<m_nSendCount<<", 已接收 = "<<m_nRecvCount<<", 丢失 = "<< m_nSendCount-m_nRecvCount<<std::endl;
        std::cout<<"往返行程的估计时间(以毫秒为单位):"<<std::endl;
        std::cout<<"   最短 = "<<nMin<<"ms,最长 = "<<nMax<<"ms ,平均= "<<nAverage<<"ms"<<std::endl;
    }

}