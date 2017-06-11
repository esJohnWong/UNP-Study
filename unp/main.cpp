#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>

#include <iostream>


#include "daytimetcpserv.h"
#include "daytimetcpcli.h"
#include "tcpserv01.h"
#include "mulitselectserv.h"
#include "tcppoll.h"
#include "epollserv.h"
#include "echoudp.h"


int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		echoserver_udp_flew_test();
	}
	else
	{
		echocli_udp_flow_test(argv[1]);
	}
}