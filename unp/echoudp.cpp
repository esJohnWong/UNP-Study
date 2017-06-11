#include "echoudp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>

#include <array>
#include <iostream>

#include "socklib.h"

void echoserver_udp()
{
	auto servfd = socket(AF_INET, SOCK_DGRAM, 0);
	ERROR_RETURN_NORET(servfd, "socket", DefErr());

	sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(9579);
	ERROR_RETURN_NORET(bind(servfd, (sockaddr*)&servAddr, sizeof(servAddr)), "bind", DefErr());

	sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	std::array<char, 200> buffer;

	while (true)
	{
		cliaddrlen = sizeof(cliaddr);
		auto n = recvfrom(servfd, buffer.data(), buffer.size(), 0, (sockaddr*)&cliaddr, &cliaddrlen);
		ERROR_RETURN_NORET(n, "recvfrom", DefErr());
		ERROR_RETURN_NORET(sendto(servfd, buffer.data(), n, 0, (sockaddr*)&cliaddr, cliaddrlen), "sendto", DefErr());
	}
}

void echocli_udp(const std::string &servAddrStr)
{
	auto clifd = socket(AF_INET, SOCK_DGRAM, 0);
	ERROR_RETURN_NORET(clifd, "socket", DefErr());

	socklen_t servaddrlen = sizeof(sockaddr_in);
	auto sockAddr =  sock_pton(AF_INET, servAddrStr);
	ERROR_RETURN_NORET(sockAddr, "sock_pton", [](decltype(sockAddr) sockAddr) {
		if (sockAddr == nullptr)
		{
			return true;
		}
		return false;
	});

	std::array<char, 200> recvBuffer;
	std::string str;

	while (std::getline(std::cin, str))
	{
		ERROR_RETURN_NORET(sendto(clifd, str.data(), str.size(), 0, (sockaddr*)sockAddr.get(), servaddrlen), "sendto", DefErr());
		auto n = recvfrom(clifd, recvBuffer.data(), recvBuffer.size(), 0, nullptr, nullptr);
		ERROR_RETURN_NORET(n, "recvfrom", DefErr());
		recvBuffer[n] = 0;
		std::cout << recvBuffer.data() << std::endl;
	}
}

void echocli_udp_connect(const std::string &servAddrStr)
{
	auto clisock = socket(AF_INET, SOCK_DGRAM, 0);
	ERROR_RETURN_NORET(clisock, "socket", DefErr());

	auto servAddr = sock_pton(AF_INET, servAddrStr);
	ERROR_RETURN_NORET(servAddr.get(), "sock_pton", NullptrErr());

	auto ret = connect(clisock, servAddr.get(), sizeof(sockaddr));
	ERROR_RETURN_NORET(ret, "connect", DefErr());

	std::string str;
	std::array<uint8_t, 200> buffer;

	while (std::getline(std::cin, str))
	{
		ret = write(clisock, str.data(), str.size());
		ERROR_RETURN_NORET(ret, "write", DefErr());

		auto n = read(clisock, buffer.data(), buffer.size());
		ERROR_RETURN_NORET(n, "read", DefErr());
		buffer[n] = 0;

		std::cout << reinterpret_cast<char *>(buffer.data()) << std::endl;
	}
}

void echocli_udp_flow_test(const std::string &servAddrStr)
{
	auto clisock = socket(AF_INET, SOCK_DGRAM, 0);
	ERROR_RETURN_NORET(clisock, "socket", DefErr());

	auto servAddr = sock_pton(AF_INET, servAddrStr);
	ERROR_RETURN_NORET(servAddr.get(), "sock_pton", NullptrErr());

	auto ret = connect(clisock, servAddr.get(), sizeof(sockaddr));
	ERROR_RETURN_NORET(ret, "connect", DefErr());

	std::array<uint8_t, 1400> buffer;

	for (auto i = 0; i < 20000; ++i)
	{
		ret = write(clisock, buffer.data(), buffer.size());
		ERROR_RETURN_NORET(ret, "write", DefErr());
	}
}

static int count = 0;

static void recv_init(int)
{
	std::cout << "count " << count << std::endl;
}

void echoserver_udp_flew_test()
{
	if (signal(SIGINT, recv_init) == SIG_ERR)
	{
		std::cout << "signal: " << strerror(errno) << std::endl;
		return;
	}

	auto servfd = socket(AF_INET, SOCK_DGRAM, 0);
	ERROR_RETURN_NORET(servfd, "socket", DefErr());

	sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(9579);
	ERROR_RETURN_NORET(bind(servfd, (sockaddr*)&servAddr, sizeof(servAddr)), "bind", DefErr());

	sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	std::array<char, 200> buffer;

	while (true)
	{
		cliaddrlen = sizeof(cliaddr);
		auto n = recvfrom(servfd, buffer.data(), buffer.size(), 0, (sockaddr*)&cliaddr, &cliaddrlen);
		ERROR_RETURN_NORET(n, "recvfrom", DefErr());
		//ERROR_RETURN_NORET(sendto(servfd, buffer.data(), n, 0, (sockaddr*)&cliaddr, cliaddrlen), "sendto", DefErr());
		++count;
	}
}