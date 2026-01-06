#pragma once
#include <iostream>
#include <memory>
#include <queue>
#include <boost/asio.hpp>

//Node管理接受发送数据长度等
const int RECVSIZE = 1024;
class MsgNode{
public:
	MsgNode(const char* msg, int total_len): _total_len(total_len), _cur_len(0){
		_msg = new char[total_len];
		memcpy(_msg, msg, total_len);
	}
	MsgNode(int total_len): _total_len(total_len), _cur_len(0){
		_msg = new char[total_len];
	}

	~MsgNode(){
		delete[] _msg;
	}
	//消息首地址，总长度，当前长度
	char* _msg;
	int _total_len;
	int _cur_len;
};

class Session
{
public:
	Session(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
	void WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<MsgNode>);
	void WriteToSocketErr(const std::string& buf);
	void Connect(const boost::asio::ip::tcp::endpoint& ep);

	void WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void WriteToSocket(const std::string& buf);

	void WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void WriteAllToSocket(const std::string& buf);

	void ReadFromSocket();
	void ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);

	void ReadAllFromSocket();
	void ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
private:
	std::queue<std::shared_ptr<MsgNode>> _send_queue;
	std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
	std::shared_ptr<MsgNode> _send_node;
	bool _send_pending;

	std::shared_ptr<MsgNode> _recv_node;
	bool _recv_pending;
};

