#include "Session.h"

void Session::Start()
{
	memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), bind(&Session::handel_read, this, std::placeholders::_1, std::placeholders::_2));
}

//监听对端发送消息
void Session::handel_read(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if(!ec){
		std::cout << "Server receive data is:" << _data << std::endl;
		boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
			std::bind(&Session::handel_write, this, std::placeholders::_1));
	}
	else{
		delete this;
	}
}


//handle_read将收到数据发送给对端，完成触发handle_write回调函数
void Session::handel_write(const boost::system::error_code& ec)
{
	if(!ec){
		memset(_data, 0, MAX_LENGTH);
		_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), std::bind(&Session::handel_read, this, std::placeholders::_1, std::placeholders::_2));
	}
	else{
		delete this;
	}
}

Server::Server(boost::asio::io_context& ioc, short port): _ioc(ioc), _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
	std::cout << "Server start success, on port:" << port << std::endl;
	start_accept();
}

void Server::start_accept()
{
	Session* new_session = new Session(_ioc);
	_acceptor.async_accept(new_session->Socket(), std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}

void Server::handle_accept(Session* new_session, const boost::system::error_code& ec)
{
	if(!ec){
		new_session->Start();
	}
	else{
		delete new_session;
	}

	start_accept();
}
