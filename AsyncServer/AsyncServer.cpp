#include <iostream>
#include <boost/asio.hpp>
#include "Session.h"

int main()
{
	try
	{
		boost::asio::io_context ioc;
		Server s(ioc, 6009);
		ioc.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception" << e.what() << std::endl;
	}

	return 0;
}
 