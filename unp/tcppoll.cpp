#include "tcppoll.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <poll.h>

#include <array>
#include <algorithm>

#include "socklib.h"

void tcppollserv()
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

	std::array<pollfd, FD_SETSIZE> client;
	for (auto &fd: client)
	{
		fd.fd = -1;
	}
	
	ssize_t maxpos = 0;
	std::array<char, 1024> buffer;
	client[0].fd = listensock;
	client[0].events = POLLRDNORM;

	while (true)
	{
		auto nready = poll(client.data(), client.size(), -1);
		ERROR_RETURN_NORET(nready, "poll", DefErr());

		if (client[0].revents & POLLRDNORM)
		{
			auto connfd = accept(listensock, nullptr, nullptr);
			ERROR_RETURN_NORET(connfd, "accept", DefErr());

			std::cout << "client " << getremotename(connfd) << std::endl;

			ssize_t newpos = -1;
			for (newpos = 0; newpos < FD_SETSIZE; ++newpos)
			{
				if (client[newpos].fd < 0)
				{
					client[newpos].fd = connfd;
					client[newpos].events = POLLRDNORM;
					break;
				}
			}

			if (newpos >= FD_SETSIZE)
			{
				std::cout << "to many clients" << std::endl;
				return;
			}

			maxpos = std::max(maxpos, newpos);

			if (--nready == 0)
			{
				continue;
			}
		}

		for (size_t i = 1; i <= maxpos; i++)
		{
			auto &fd = client[i];
			if (fd.fd < 0)
			{
				continue;
			}

			if (fd.revents & (POLLRDNORM | POLLERR))
			{
				auto n = read(fd.fd, buffer.data(), buffer.size());
				ERROR_RETURN_NORET(n, "read", DefErr());

				if (n == 0)
				{
					close(fd.fd);
					client[i].fd = -1;
				}
				else
				{
					ERROR_RETURN_NORET(writen(fd.fd, buffer.data(), n), "write", DefErr());
				}

				if (--nready <= 0)
				{
					break;
				}
			}
		}
	}
}