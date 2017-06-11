#include "socklib.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

#include <utility>

std::string sock_ntop(const struct sockaddr *sa)
{
	switch (sa->sa_family)
	{
	case AF_INET:
	{
		auto sin = reinterpret_cast<const sockaddr_in *>(sa);
		char buf[INET_ADDRSTRLEN] = { 0 };

		if (inet_ntop(AF_INET, &sin->sin_addr, buf, INET_ADDRSTRLEN) == nullptr)
		{
			return "";
		}

		auto port = ntohs(sin->sin_port);

		std::string str = buf;
		str += ":";
		str += std::to_string(port);
		return str;
	}
	}

	return "";
}

std::shared_ptr<sockaddr> sock_pton(sa_family_t family, const std::string &addr)
{
	switch (family)
	{
	case AF_INET:
	{
		/**
		 *	将192.168.1.1:123类型的字符串转换为地址和端口组成的pair
		 */
		auto split = [](const std::string &str, const std::string &sub = ":")
		{
			auto subPos = str.find(sub);
			if (subPos == std::string::npos)
			{
				return std::make_pair(str, 0);
			}

			return std::make_pair(str.substr(0, subPos), std::stoi(str.substr(subPos + 1)));
		};

		auto addrPort = split(addr);
		sockaddr_in sockAddrIn;
		bzero(&sockAddrIn, sizeof(sockAddrIn));
		sockAddrIn.sin_family = AF_INET;
		sockAddrIn.sin_port = htons(static_cast<uint16_t>(addrPort.second));

		if (inet_pton(AF_INET, addrPort.first.c_str(), &sockAddrIn.sin_addr) <=0)
		{
			return nullptr;
		}
		
		/**
		 *	构建一个sockaddr类型的智能指针
		 */
		auto retAddr = new sockaddr;
		bzero(retAddr, sizeof(sockaddr));
		bcopy(&sockAddrIn, retAddr, sizeof(sockAddrIn));
		return std::shared_ptr<sockaddr>(retAddr);
	}
	}

	return nullptr;
}

ssize_t readn(int fd, void *buf, size_t n)
{
	auto ptr = reinterpret_cast<byte_t *>(buf);
	auto nleft = n;
	ssize_t nread = 0;

	while (nleft > 0)
	{
		nread = read(fd, ptr, nleft);
		if (nread < 0)
		{
			if (errno == EINTR)			// 被信号中断
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)			// 没有数据可以再读取了
		{
			break;
		}

		nleft -= nread;
		ptr += nread;
	}

	return n - nleft;
}

ssize_t writen(int fd, const void *buf, size_t n)
{
	size_t nleft = n;
	ssize_t nwrite = 0;
	auto ptr = reinterpret_cast<const byte_t *>(buf);

	while (nleft > 0)
	{
		if ((nwrite = write(fd, ptr, nleft)) <= 0)
		{
			if (nwrite < 0 && errno == EINTR)
			{
				nwrite = 0;
			}
			else
			{
				return -1;
			}
		}

		nleft -= nwrite;
		ptr += nwrite;
	}

	return n;
}

ssize_t readintr(int fd, void *buf, size_t n)
{
	while (true)
	{
		auto readsize = read(fd, buf, n);
		if (readsize < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}

			return -1;
		}

		return readsize;
	}

	return 0;
}

/**
 *	读取一个字符
 *	预先读取一批数据到缓冲中
 */
static int readch(read_handle &handle)
{
	if (handle.readcnt <= 0)
	{
		handle.readcnt = readintr(handle.fd, handle.buf, handle.MAXLEN);
		if (handle.readcnt < 0)
		{
			return -1;
		}
		else if (handle.readcnt == 0)
		{
			return 0;
		}

		handle.readptr = handle.buf;
	}

	auto ch = *handle.readptr++;
	--handle.readcnt;
	return ch;
}

ssize_t readline(read_handle &handle, std::string &line)
{
	line.clear();

	while (true)
	{
		auto ch = readch(handle);
		if (ch < 0)
		{
			return -1;
		}
		else if (ch == 0)
		{
			break;
		}

		if (ch == '\n')
		{
			break;
		}

		line += static_cast<char>(ch);
	}

	return line.size();
}

std::string getlocalname(int sockfd)
{
	sockaddr localaddr;
	socklen_t locallen = sizeof(sockaddr);
	bzero(&localaddr, locallen);
	getsockname(sockfd, &localaddr, &locallen);
	return sock_ntop(&localaddr);
}

std::string getremotename(int sockfd)
{
	sockaddr remoteaddr;
	socklen_t remotelen = sizeof(remoteaddr);
	bzero(&remoteaddr, remotelen);
	getpeername(sockfd, &remoteaddr, &remotelen);
	return sock_ntop(&remoteaddr);
}