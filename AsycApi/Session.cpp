#include "Session.h"

//
Session::Session(std::shared_ptr<boost::asio::ip::tcp::socket> socket): _socket(socket), _send_pending(false), _recv_pending(false)
{

}

void Session::WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<MsgNode> msg_node)
{
	//当传输数据没有发送完全(跟tcp缓冲区大小有关)，递归调用
	if(bytes_transferred + msg_node->_cur_len < msg_node->_total_len){
		//更新当前长度
		_send_node->_cur_len += bytes_transferred;
		this->_socket->async_write_some(boost::asio::buffer(_send_node->_msg + _send_node->_cur_len, _send_node->_total_len - _send_node->_cur_len),
			std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
	}
}

void Session::WriteToSocketErr(const std::string& buf)
{
	//生命周期管理
	_send_node = std::make_shared<MsgNode>(buf.c_str(), buf.length());
	//异步发送，buffer给首地址及长度，并且绑定WriteCallBackErr回调函数
	this->_socket->async_write_some(boost::asio::buffer(_send_node->_msg, _send_node->_total_len), std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
}

void Session::Connect(const boost::asio::ip::tcp::endpoint& ep)
{
	_socket->connect(ep);
}

void Session::WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec.value() != 0) {
		std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
		return;
	}
	auto& send_data = _send_queue.front();
	send_data->_cur_len += bytes_transferred;
	//未发送完成继续调用
	if(send_data->_cur_len < send_data->_total_len){
		this->_socket->async_write_some(boost::asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len),
			std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
		return;
	}
	//若队列空，停止，非空继续发送下一条
	_send_queue.pop();
	if(_send_queue.empty()){
		_send_pending = false;
	}
	if(!_send_queue.empty()){
		auto send_data = _send_queue.front();
		this->_socket->async_write_some(boost::asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len),
			std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void Session::WriteToSocket(const std::string& buf)
{
	_send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
	if(_send_pending){
		return;
	}
	this->_socket->async_write_some(boost::asio::buffer(buf), std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_send_pending = true;
}

void Session::WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec.value() != 0) {
		std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
		return;
	}

	_send_queue.pop();
	if(_send_queue.empty()){
		_send_pending = false;
	}
	if(!_send_queue.empty()){
		auto& send_data = _send_queue.front();
		this->_socket->async_send(boost::asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len),
			std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void Session::WriteAllToSocket(const std::string& buf)
{
	_send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
	if(_send_pending){
		return;
	}
	//解决async_write_some多次回调问题
	this->_socket->async_send(boost::asio::buffer(buf), std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_send_pending = true;
}

void Session::ReadFromSocket()
{
	if(_recv_pending){
		return;
	}
	_recv_node = std::make_shared<MsgNode>(RECVSIZE);
	_socket->async_read_some(boost::asio::buffer(_recv_node->_msg, _recv_node->_total_len), std::bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_recv_pending = true;
}

void Session::ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	_recv_node->_cur_len += bytes_transferred;
	//没读完继续处理
	if(_recv_node->_cur_len < _recv_node->_total_len){
		_socket->async_read_some(boost::asio::buffer(_recv_node->_msg + _recv_node->_cur_len, _recv_node->_total_len - _recv_node->_cur_len),
			std::bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
		return;
	}
	_send_pending = false;
	_recv_node = nullptr;
}

void Session::ReadAllFromSocket()
{
	if(_recv_pending){
		return;
	}
	_recv_node = std::make_shared<MsgNode>(RECVSIZE);
	_socket->async_receive(boost::asio::buffer(_recv_node->_msg, _recv_node->_total_len), std::bind(&Session::ReadAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_recv_pending = true;
}

void Session::ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	_recv_node->_cur_len += bytes_transferred;
	_recv_pending = false;
	_send_node = nullptr;
}
