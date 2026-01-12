#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <queue>
#include <sstream>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#define HEAD_LENGTH 2
#define MAX_SENDQUE 1000

class CServer;

//class MsgNode{
//	friend class CSession;
//public:
//	MsgNode(char *msg, int max_len){
//		_data = new char[max_len];
//		memcpy(_data, msg, max_len);
//	}
//	~MsgNode(){
//		delete[] _data;
//	}
//private:
//	int _cur_len;
//	int _max_len;
//	char* _data;
//};

//解决粘包问题
class MsgNode{
	friend class CSession;
public:
	MsgNode(char* msg, int max_len): _total_len(max_len + HEAD_LENGTH), _cur_len(0){
		_data = new char[_total_len + 1]();
		//服务器发送数据构造消息节点时，从本地字节序转为网络字节序
		int max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
		memcpy(_data, &max_len_host, HEAD_LENGTH);
		memcpy(_data + HEAD_LENGTH, msg, max_len);
		_data[_total_len] = '\0';
	}
	MsgNode(short max_len): _total_len(max_len), _cur_len(0){
		_data = new char[_total_len + 1]();
	}
	~MsgNode(){
		delete[] _data;
	}
	void Clear(){
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}
private:
	int _cur_len;
	int _total_len;
	char* _data;
};


//处理客户端消息收发
class CSession :public std::enable_shared_from_this<CSession>
{
public:

	CSession(boost::asio::io_context& io_context, CServer* server) :
		_socket(io_context), _server(server), _b_head_parse(false) {
		boost::uuids::uuid  a_uuid = boost::uuids::random_generator()();
		_uuid = boost::uuids::to_string(a_uuid);
		_recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
	}
	~CSession() {
		std::cout << "session destruct delete this " << this << std::endl;
	}
	boost::asio::ip::tcp::socket& Socket() {
		return _socket;
	}

	void Start();
	std::string& GetUuid();
	//发送队列
	void Send(char* msg, int max_length);
	void PrintRecvData(char* data, int length);

private:
	void handel_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> _self_shared);
	void handel_write(const boost::system::error_code& ec, std::shared_ptr<CSession> _self_shared);
	boost::asio::ip::tcp::socket _socket;
	enum { MAX_LENGTH = 1024 };
	char _data[MAX_LENGTH];
	CServer* _server;
	std::string _uuid;

	//加锁
	std::queue<std::shared_ptr<MsgNode>> _send_que;
	std::mutex _send_lock;

	//粘包问题，收到的消息结构
	std::shared_ptr<MsgNode> _recv_msg_node;
	bool _b_head_parse;
	//收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;

};