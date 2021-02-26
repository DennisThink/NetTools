#ifndef _DENNIS_THINK_ICMP_HEADER_H_
#define _DENNIS_THINK_ICMP_HEADER_H_
#include "ip_header.h"
//ICMP头
struct ICMP_head
{
    uint8_t type;//类型
    uint8_t code;//代码
    uint16_t checkSum;//校验和
    uint16_t ident;//进程标识符
    uint16_t seqNum;//序号
};
//ICMP回显请求报文(发送用)
struct ICMP_Req
{
    ICMP_head icmphead;//头部
    uint32_t timeStamp;//时间戳
    char     data[32];//数据
};
//ICMP应答报文(接收用)
struct ICMP_Reply
{
    IPhead iphead;//IP头
    ICMP_Req icmpanswer;//ICMP报文
    char data[1024];//应答报文携带的数据缓冲区
};
#endif