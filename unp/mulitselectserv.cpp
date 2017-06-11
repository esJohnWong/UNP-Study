#include "mulitselectserv.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

#include <array>
#include <algorithm>

#include "socklib.h"

void multiselectserv()
{
	sockaddr_in seraddr;
	socklen_t addrlen = sizeof(seraddr);
	bzero(&seraddr, addrlen);
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(9877);
	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	auto listensock = socket(AF_INET, SOCK_STREAM, 0);
	ERROR_RETURN_NORET(listensock, "socket", DefErr());
	ERROR_RETURN_NORET(bind(listensock, (sockaddr*)&seraddr, addrlen), "bind", DefErr());
	ERROR_RETURN_NORET(listen(listensock, 5), "listen", DefErr());

	std::array<int, FD_SETSIZE> client;
	client.fill(-1);

	fd_set rset, allset;
	FD_ZERO(&rset);
	FD_SET(listensock, &rset);
	allset = rset;
	auto maxfd = listensock;
	ssize_t maxpos = 0;
	std::array<char, 1024> buffer;

	while (true)
	{
		rset = allset;
		auto nready = select(maxfd + 1, &rset, nullptr, nullptr, nullptr);
		ERROR_RETURN_NORET(nready, "select", DefErr());

		if (FD_ISSET(listensock, &rset))
		{
			auto connfd = accept(listensock, nullptr, nullptr);
			ERROR_RETURN_NORET(connfd, "accept", DefErr());

			std::cout << "client " <<  getremotename(connfd) << std::endl;

			ssize_t newpos = -1;
			for (newpos = 0; newpos < FD_SETSIZE; ++newpos)
			{
				if (client[newpos] < 0)
				{
					client[newpos] = connfd;
					break;
				}
			}

			if (newpos >= FD_SETSIZE)
			{
				std::cout << "to many clients" << std::endl;
				return;
			}

			FD_SET(connfd, &allset);

			maxfd = std::max(maxfd, connfd);
			maxpos = std::max(maxpos, newpos);

			if (--nready == 0)
			{
				continue;
			}
		}

		for (size_t i = 0; i <= maxpos; i++)
		{
			auto fd = client[i];
			if (fd < 0)
			{
				continue;
			}

			if (FD_ISSET(fd, &rset))
			{
				auto n = read(fd, buffer.data(), buffer.size());
				ERROR_RETURN_NORET(n, "read", DefErr());
				
				if (n == 0)
				{
					close(fd);
					FD_CLR(fd, &allset);
					client[i] = -1;
				}
				else
				{
					ERROR_RETURN_NORET(writen(fd, buffer.data(), n), "write", DefErr());
				}

				if (--nready <= 0)
				{
					break;
				}
			}
		}
	}
}