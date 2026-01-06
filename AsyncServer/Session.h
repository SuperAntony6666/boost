#pragma once
#include <iostream>
#include <boost/asio.hpp>


//处理客户端消息收发
class Session
{
public:
	
	Session(boost::asio::io_context& ioc) :_socket(ioc) {}
	boost::asio::ip::tcp::socket& Socket() {
		return _socket;
	}

	void Start();
private:
	void handel_read(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void handel_write(const boost::system::error_code& ec);
	boost::asio::ip::tcp::socket _socket;
	enum {MAX_LENGTH = 1024};
	char _data[MAX_LENGTH];
};


//服务器接受连接管理
class Server{
public:
	Server(boost::asio::io_context& ioc, short port);
private:
	void start_accept();
	void handle_accept(Session* new_session, const boost::system::error_code& ec);
	boost::asio::io_context& _ioc;
	boost::asio::ip::tcp::acceptor _acceptor;
};