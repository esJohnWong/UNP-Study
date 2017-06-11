#pragma once

#include <sys/socket.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <memory>
#include <iostream>

/**
 *	�ֽ�����
 */
using byte_t = uint8_t;

/**
 *	��sockaddr�ṹת��Ϊ192.168.1.1:123���͵��ַ���
 */
std::string sock_ntop(const struct sockaddr *sa);

/**
 *	��192.168.1.1:123���͵��ַ�����ַת��Ϊsockaddr�ṹ
 */
std::shared_ptr<sockaddr> sock_pton(sa_family_t family, const std::string &addr);

/**
 *	��fd�ж�ȡn���ֽڣ�������-1�����򷵻ض�ȡ�����ֽ���
 */
ssize_t readn(int fd, void *buf, size_t n);

/**
 *	��buf�е�n�ֽ�д�뵽fd�У��ɹ�����n�����򷵻�-1
 */
ssize_t writen(int fd, const void *buf, size_t n);

/**
 * �Զ������ж�
 */
ssize_t readintr(int fd, void *buf, size_t n);

/**
 * ��fd�󶨵Ĵ�����ľ��
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
 *	��ȡһ��
 */
ssize_t readline(read_handle &handle, std::string &line);

/**
 *	Ĭ�ϴ����ж϶���
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
 *	testʹ��һԪν��fnΪ�������������Ϣmsg��ִ��return��������ֵ
 */
#define ERROR_RETURN_NORET(test, msg, fn) {\
	if (fn(test))\
	{\
		std::cerr << msg << ": " << strerror(errno) << std::endl;\
		return;\
	}\
}	

/**
 *	testʹ��һԪν��fnΪ�������������Ϣmsg��ִ��return������ֵret
 */
#define ERROR_RETURN_RET(test, msg, fn, ret) {\
	if (fn(test))\
	{\
		std::cerr << msg << ": " << strerror(errno) << std::endl;\
		return ret;\
	}\
}

/**
 *	��ȡ�׽���sockfd�ϱ��غͶԶ˵�ַ�Ͷ˿�
 */
std::string getlocalname(int sockfd);
std::string getremotename(int sockfd);