#ifndef _DENNIS_THINK_IP_HEADER_H_
#define _DENNIS_THINK_IP_HEADER_H_
struct IPhead
{
    //这里使用了C语言的位域，也就是说像version变量它的大小在内存中是占4bit，而不是8bit
    uint8_t     version : 4; //IP协议版本
    uint8_t     headLength : 4;//首部长度
    uint8_t     serverce;//区分服务
    uint16_t    totalLength;//总长度
    uint16_t    flagbit;//标识
    uint16_t    flag : 3;//标志
    uint16_t    fragmentOffset : 13;//片偏移
    char        timetoLive;//生存时间（跳数）
    uint8_t     protocol;//使用协议
    uint16_t    headcheckSum;//首部校验和
    uint32_t    srcIPadd;//源IP
    uint32_t    dstIPadd;//目的IP
    //可选项和填充我就不定义了
};
#endif