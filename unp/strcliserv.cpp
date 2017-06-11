#include "strcliserv.h"

#include <stdio.h>
#include <error.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include <iostream>
#include <algorithm>

#include "socklib.h"

void str_cli06(int sockfd)
{
	std::string str;
	std::string strread;
	fd_set readset;
	read_handle handle(sockfd);
	FD_ZERO(&readset);

	while (true)
	{
		FD_SET(STDIN_FILENO, &readset);
		FD_SET(sockfd, &readset);
		auto maxfd = std::max(STDIN_FILENO, sockfd) + 1;
		
		if (select(maxfd, &readset, nullptr, nullptr, nullptr) < 0)
		{
			std::cerr << "select " << strerror(errno) << std::endl;
			return;
		}
		

		if (FD_ISSET(sockfd, &readset))
		{
			if (readline(handle, strread) < 0)
			{
				std::cerr << "readline " << strerror(errno) << std::endl;
				return;
			}

			std::cout << strread << std::endl;
		}

		if (FD_ISSET(STDIN_FILENO, &readset))
		{
			std::getline(std::cin, str);
			str += "\n";

			if (writen(sockfd, str.data(), str.length()) < 0)
			{
				std::cerr << "writen " << strerror(errno) << std::endl;
				return;
			}
		}
	}
}