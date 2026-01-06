#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>

std::set<std::shared_ptr<std::thread>> thread_set;

const int MAX_LENGTH = 1024;


//share_ptr管理sock
void session(std::shared_ptr<boost::asio::ip::tcp::socket> sock) {
	try
	{
		for (; ;) {
			char data[MAX_LENGTH];
			memset(data, '\0', MAX_LENGTH);
			boost::system::error_code error;
			std::size_t length = sock->read_some(boost::asio::buffer(data, MAX_LENGTH), error);
			if (error == boost::asio::error::eof) {
				std::cout << "connection closed by peer" << std::endl;
				break;
			}
			else if (error) {
				throw boost::system::system_error(error);
			}
			std::cout << "receive from " << sock->remote_endpoint().address().to_string() << std::endl;
			std::cout << "receive message is " << data << std::endl;
			//回传信息
			boost::asio::write(*sock, boost::asio::buffer(data, length));
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
	}
}


//server函数根据服务器ip和端口创建服务器acceptor用来接收数据，用socket接收新的连接，然后为这个socket创建session。
void server(boost::asio::io_context& ioc, unsigned short port) {
	boost::asio::ip::tcp::acceptor acc(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
	for (; ; ) {
		std::shared_ptr<boost::asio::ip::tcp::socket> socket(new boost::asio::ip::tcp::socket(ioc));
		acc.accept(*socket);
		auto t = std::make_shared<std::thread>(session, socket);
		thread_set.insert(t);
	}
}


int main()
{
	//等待子线程退出完后，主线程再退出
	try
	{
		boost::asio::io_context ioc;
		server(ioc, 6009);
		for (auto& t : thread_set) {
			t->join();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
	}
}
