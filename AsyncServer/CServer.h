#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <string>

class CSession;

//服务器接受连接管理
class CServer {
public:
	CServer(boost::asio::io_context& ioc, short port);
	void ClearSession(std::string uuid);
private:
	void start_accept();
	void handle_accept(std::shared_ptr<CSession> new_session, const boost::system::error_code& ec);
	boost::asio::io_context& _ioc;
	boost::asio::ip::tcp::acceptor _acceptor;
	std::map <std::string, std::shared_ptr<CSession>> _session;
};
