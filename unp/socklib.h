#pragma once

#include <sys/socket.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <memory>
#include <iostream>

/**
 *	字节类型
 */
using byte_t = uint8_t;

/**
 *	将sockaddr结构转换为192.168.1.1:123类型的字符串
 */
std::string sock_ntop(const struct sockaddr *sa);

/**
 *	将192.168.1.1:123类型的字符串地址转换为sockaddr结构
 */
std::shared_ptr<sockaddr> sock_pton(sa_family_t family, const std::string &addr);

/**
 *	从fd中读取n个字节，出错返回-1，否则返回读取到的字节数
 */
ssize_t readn(int fd, void *buf, size_t n);

/**
 *	将buf中的n字节写入到fd中，成功返回n，否则返回-1
 */
ssize_t writen(int fd, const void *buf, size_t n);

/**
 * 自动处理中断
 */
ssize_t readintr(int fd, void *buf, size_t n);

/**
 * 与fd绑定的带缓冲的句柄
 */
struct read_handle
{
	constexpr static int MAXLEN = 4096;

	int fd = 0;
	ssize_t	readcnt = 0;
	char *readptr = nullptr;
	char buf[MAXLEN];

	read_handle(int fd_) 
		: fd(fd_)
	{}
};

/**
 *	读取一行
 */
ssize_t readline(read_handle &handle, std::string &line);

/**
 *	默认错误判断对象
 */
struct DefErr 
{
	inline bool operator()(long ret)
	{
		if (ret < 0)
		{
			return true;
		}
		return false;
	}
};

struct NullptrErr
{
	inline bool operator()(void *ptr)
	{
		if (ptr == nullptr)
		{
			return true;
		}

		return false;
	}
};
 
/**
 *	test使得一元谓词fn为真则输出错误消息msg，执行return不带返回值
 */
#define ERROR_RETURN_NORET(test, msg, fn) {\
	if (fn(test))\
	{\
		std::cerr << msg << ": " << strerror(errno) << std::endl;\
		return;\
	}\
}	

/**
 *	test使得一元谓词fn为真则输出错误消息msg，执行return带返回值ret
 */
#define ERROR_RETURN_RET(test, msg, fn, ret) {\
	if (fn(test))\
	{\
		std::cerr << msg << ": " << strerror(errno) << std::endl;\
		return ret;\
	}\
}

/**
 *	获取套接字sockfd上本地和对端地址和端口
 */
std::string getlocalname(int sockfd);
std::string getremotename(int sockfd);