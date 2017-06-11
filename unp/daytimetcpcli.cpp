#include "daytimetcpcli.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "socklib.h"

int daytime_cli(const std::string &addr)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		std::cerr << "socket " << strerror(errno) << std::endl;
		return -1;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(13);

	if (inet_pton(AF_INET, addr.c_str(), &servaddr.sin_addr) <= 0)
	{
		std::cerr << "inet_pton " << strerror(errno) << std::endl;
		return -1;
	}

	if (connect(fd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "connect " << strerror(errno) << std::endl;
		return -1;
	}

	sockaddr addrtmp;
	socklen_t addrlen = sizeof(addrtmp);
	bzero(&addrtmp, addrlen);
	getsockname(fd, &addrtmp, &addrlen);

	std::cout << "client connect at " << sock_ntop(&addrtmp) << std::endl;

	static constexpr int MAXLEN = 1000;
	ssize_t n = 0;
	char buf[100] = { 0 };
	while ((n = read(fd, buf, MAXLEN)) > 0)
	{
		buf[n] = 0;

		std::cout << buf << std::endl;
	}

	if (n < 0)
	{
		std::cerr << "read " << strerror(errno) << std::endl;
		return -1;
	}
	
	close(fd);
	return 0;
}