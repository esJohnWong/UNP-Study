#include "epollserv.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/epoll.h>
#include <signal.h>

#include <array>
#include <algorithm>

#include "socklib.h"

void epollserv()
{
	auto epfd = epoll_create1(0);
	ERROR_RETURN_NORET(epfd, "epoll_create1", DefErr());

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

	epoll_event listenev;
	listenev.events = EPOLLIN;
	listenev.data.fd = listensock;
	ERROR_RETURN_NORET(epoll_ctl(epfd, EPOLL_CTL_ADD, listensock, &listenev), "epoll_ctl", DefErr());

	std::array<epoll_event, FD_SETSIZE> evlist;
	std::array<char, 1024> buffer;
	sigset_t fullsig;
	sigfillset(&fullsig);

	while (true)
	{
		auto nready = epoll_pwait(epfd, evlist.data(), int(evlist.size()), -1, &fullsig);
		ERROR_RETURN_NORET(nready, "epoll_wait", DefErr());

		for (int i = 0; i < nready; i++)
		{
			if (evlist[i].data.fd == listensock)
			{
				auto connfd = accept(listensock, nullptr, nullptr);
				ERROR_RETURN_NORET(connfd, "accept", DefErr());

				std::cout << "client " << getremotename(connfd) << std::endl;
				
				epoll_event connev;
				// 设置有数据或者远端close
				connev.events = EPOLLIN | EPOLLRDHUP;
				connev.data.fd = connfd;

				ERROR_RETURN_NORET(epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &connev), "epoll_ctl", DefErr());
				continue;
			}

			// 判断出错事件或者关闭事件必须在判断EPOLLIN前，如果远端close则EPOLLIN也设置
			if (evlist[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
			{
				std::cout << "closeing " << getremotename(evlist[i].data.fd) << std::endl;
				close(evlist[i].data.fd);
				continue;
			}

			if (evlist[i].events & EPOLLIN)
			{
				auto n = read(evlist[i].data.fd, buffer.data(), buffer.size());
				ERROR_RETURN_NORET(n, "read", [](long fd) {
					if (fd <= 0) {
						return true;
					}
					return false;
				});

				ERROR_RETURN_NORET(write(evlist[i].data.fd, buffer.data(), n), "write", DefErr());
				continue;
			}
		}
	}
}
