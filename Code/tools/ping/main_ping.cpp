/**
 * @file main_ping.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief This is the main file for ping writen by DennisMi
 * @version 0.1
 * @date 2021-03-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <iostream>
#include "ping.h"
void print_help() 
{
    
    std::cout<<"用法: ping [-t] [-a] [-n count] [-l size] [-f] [-i TTL] [-v TOS]"<<std::endl;
    std::cout<<"           [-r count] [-s count] [[-j host-list] | [-k host-list]]"<<std::endl;
    std::cout<<"           [-w timeout] [-R] [-S srcaddr] [-4] [-6] target_name"<<std::endl;

    std::cout<<"选项:"<<std::endl;
    std::cout<<" -t            Ping 指定的主机，直到停止。"<<std::endl;
    std::cout<<"               若要查看统计信息并继续操作 - 请键入 Control-Break；"<<std::endl;
    std::cout<<"               若要停止 - 请键入 Control-C。"<<std::endl;
    std::cout<<"-a             将地址解析成主机名。"<<std::endl;
    std::cout<<"-n count       要发送的回显请求数。"<<std::endl;
    std::cout<<"-l size        发送缓冲区大小。"<<std::endl;
    std::cout<<"-f             在数据包中设置“不分段”标志(仅适用于 IPv4)。"<<std::endl;
    std::cout<<"-i TTL         生存时间。"<<std::endl;
    std::cout<<"-v TOS         服务类型(仅适用于 IPv4。该设置已不赞成使用，且"<<std::endl;
    std::cout<<"               对 IP 标头中的服务字段类型没有任何影响)。"<<std::endl;
    std::cout<<"-r count       记录计数跃点的路由(仅适用于 IPv4)。"<<std::endl;
    std::cout<<"-s count       计数跃点的时间戳(仅适用于 IPv4)。"<<std::endl;
    std::cout<<"-j host-list   与主机列表一起的松散源路由(仅适用于 IPv4)。"<<std::endl;
    std::cout<<"-k host-list   与主机列表一起的严格源路由(仅适用于 IPv4)。"<<std::endl;
    std::cout<<"-w timeout     等待每次回复的超时时间(毫秒)。"<<std::endl;
    std::cout<<"-R             同样使用路由标头测试反向路由(仅适用于 IPv6)。"<<std::endl;
    std::cout<<"-S srcaddr     要使用的源地址。"<<std::endl;
    std::cout<<"-4             强制使用 IPv4。"<<std::endl;
    std::cout<<"-6             强制使用 IPv6。"<<std::endl;
}

void print_version()
{

}
int main(int argc,char * argv[])
{
    if(1 == argc)
    {
        print_help();
        return 0;
    }
    dennisthink::DtPing ping;
    ping.doPing(argv[1]);
    return 0;
}