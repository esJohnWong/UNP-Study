#include "chapter09.h"

#include <netdb.h>

#include <istream>

#include "socklib.h"

chapter09::chapter09()
{
}


chapter09::~chapter09()
{
}

void chapter09::getHostByName(const std::string &host)
{
	auto phost = gethostbyname(host.c_str());
	if (phost == nullptr)
	{
		std::cerr << hstrerror(h_errno) << std::endl;
		return;
	}

	std::cout << "alias: ";

	for (auto alias = phost->h_aliases; *alias != nullptr; ++alias)
	{

	}
}
