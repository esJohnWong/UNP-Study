#include "tcpserv01.h"

#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <iostream>

#include "socklib.h"
#include "strcliserv.h"

static void str_echo(int sockfd)
{
	constexpr static int MAXLEN = 4096;
	char buf[MAXLEN] = { 0 };

	while (true)
	{
		auto n = read(sockfd, buf, MAXLEN);
		if (n < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			
			std::cerr << "read " << strerror(errno) << std::endl;
			return;
		}
		else if (n > 0)
		{
			if (writen(sockfd, buf, n) < 0)
			{
				std::cerr << "writen " << strerror(errno) << std::endl;
				return ;
			}

			buf[n] = 0;
			std::cout << "write " << buf << std::endl;
		}
		else
		{
			std::cout << "read 0" << std::endl;
			return;
		}
	}
}

void chldsig(int signo)
{
	while (waitpid(-1, nullptr, WNOHANG));
}

int tcpserv01()
{
	struct sigaction act;
	act.sa_handler = chldsig;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &act, nullptr) < 0)
	{
		std::cerr << "sigaction " << strerror(errno) << std::endl;
		return -1;
	}

	auto listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		std::cerr << "socket " << strerror(errno) << std::endl;
		return -1;
	}

	auto servAddr = sock_pton(AF_INET, "0.0.0.0:9877");
	if (servAddr == nullptr)
	{
		std::cerr << "sock_pton " << strerror(errno) << std::endl;
		return -1;
	}

	if (bind(listenfd, servAddr.get(), sizeof(sockaddr_in)) < 0)
	{
		std::cerr << "bind " << strerror(errno) << std::endl;
		return -1;
	}

	if (listen(listenfd, 8) < 0)
	{
		std::cerr << "listen " << strerror(errno) << std::endl;
		return -1;
	}

	socklen_t cliaddrlen = 0;
	sockaddr_in cliaddr;
	pid_t childpid = 0;

	while (true)
	{
		cliaddrlen = sizeof(socklen_t);
		bzero(&cliaddr, cliaddrlen);

		auto connfd = accept(listenfd, (sockaddr *)&cliaddr, &cliaddrlen);
		if (connfd < 0)
		{
			std::cerr << "accept " << strerror(errno) << std::endl;
			return -1;
		}

		std::cout << "child " << sock_ntop(reinterpret_cast<sockaddr*>(&cliaddr)) << std::endl;

		if ((childpid = fork()) == 0)
		{
			sockaddr localaddr;
			socklen_t locallen = sizeof(sockaddr);
			bzero(&localaddr, locallen);
			getsockname(connfd, &localaddr, &locallen);

			sockaddr remoteaddr;
			socklen_t remotelen = sizeof(remoteaddr);
			bzero(&remoteaddr, remotelen);
			getpeername(connfd, &remoteaddr, &remotelen);

			std::cout << "local addr " << sock_ntop(&localaddr) << std::endl;
			std::cout << "remote addr " << sock_ntop(&remoteaddr) << std::endl;

			close(listenfd);
			str_echo(connfd);
			exit(0);
		}
		else if (childpid > 0)
		{
			close(connfd);
		}
		else
		{
			std::cerr << "fork " << strerror(errno) << std::endl;
			return -1;
		}
	}

	return 0;
}

static void str_cli(int sockfd)
{
	std::string str;
	std::string strread;

	read_handle handle(sockfd);

	while (std::getline(std::cin, str))
	{
		str += "\n";

		if (writen(sockfd, str.data(), str.length()) < 0)
		{
			std::cerr << "writen " << strerror(errno) << std::endl;
			return;
		}

		if (readline(handle, strread) < 0)
		{
			std::cerr << "readline " << strerror(errno) << std::endl;
			return;
		}

		std::cout << strread << std::endl;
	}
}

int tcpcli01(const std::string &servaddr)
{
	/** 
	 * 防止服务器挂掉后出现进程被终止
	 */
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	act.sa_flags = SA_INTERRUPT;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGPIPE, &act, nullptr) < 0)
	{
		std::cerr << "sigaction " << strerror(errno) << std::endl;
		return -1;
	}

	auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		std::cerr << "socket " << strerror(errno) << std::endl;
		return -1;
	}

	auto addr = sock_pton(AF_INET, servaddr);
	if (addr == nullptr)
	{
		std::cerr << "sock_pton " << strerror(errno) << std::endl;
		return -1;
	}

	if (connect(sockfd, addr.get(), sizeof(sockaddr_in)) < 0)
	{
		std::cerr << "connect " << strerror(errno) << std::endl;
		return -1;
	}

	sockaddr localaddr;
	socklen_t localaddrlen = sizeof(localaddr);
	bzero(&localaddr, sizeof(localaddr));
	getsockname(sockfd, &localaddr, &localaddrlen);

	std::cout << "local addr " << sock_ntop(&localaddr) << std::endl;

	sockaddr remoteaddr;
	socklen_t remotelen = sizeof(remoteaddr);
	bzero(&remoteaddr, remotelen);
	getpeername(sockfd, &remoteaddr, &remotelen);
	std::cout << "remote addr " << sock_ntop(&remoteaddr) << std::endl;

	str_cli06(sockfd);

	exit(0);
}