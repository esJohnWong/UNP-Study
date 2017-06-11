#include "daytimetcpserv.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>

#include "socklib.h"


int daytime_ser()
{
	auto listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		std::cerr << "socket " << strerror(errno) << std::endl;
		return -1;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(13);

	if (bind(listenfd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "bind " << strerror(errno) << std::endl;
		return -1;
	}


	sockaddr addrtmp;
	socklen_t addrlen = sizeof(addrtmp);
	bzero(&addrtmp, sizeof(addrtmp));
	getsockname(listenfd, &addrtmp, &addrlen);

	std::cout << "server bind at: " << sock_ntop(&addrtmp) << std::endl;

	if (listen(listenfd, 5) < 0)
	{
		std::cerr << "listen " << strerror(errno) << std::endl;
		return -1;
	}

	char buff[100];

	while (true)
	{
		auto connfd = accept(listenfd, nullptr, nullptr);
		
		addrlen = sizeof(addrtmp);
		bzero(&addrtmp, sizeof(addrtmp));
		getpeername(connfd, &addrtmp, &addrlen);

		std::cout << "connect at " << sock_ntop(&addrtmp) << std::endl;

		if (connfd < 0)
		{
			std::cerr << "accept " << strerror(errno) << std::endl;
			return -1;
		}

		auto curtime = time(nullptr);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&curtime));
		if (write(connfd, buff, strlen(buff)) < 0)
		{
			std::cerr << "write " << strerror(errno) << std::endl;
			return -1;
		}
		close(connfd);
	}

	return 0;
}
