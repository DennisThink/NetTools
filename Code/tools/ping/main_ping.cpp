#include <iostream>
#include "ping.h"
int main(int argc,char * argv[])
{
    std::cout<<"ping program by DennisThink"<<std::endl;
    dennisthink::DtPing ping;
    ping.doPing(argv[1]);
    return 0;
}