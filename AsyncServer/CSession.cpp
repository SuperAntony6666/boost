#include "CSession.h"
#include "CServer.h"
#include <iomanip>


void CSession::Start()
{
	memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), bind(&CSession::handel_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

std::string& CSession::GetUuid()
{
	return _uuid;
}


//void CSession::Send(char* msg, int max_length)
//{
//	bool pending = false;
//	std::lock_guard<std::mutex> lock(_send_lock);
//	if(_send_que.size() > 0){
//		pending = true;
//	}
//	_send_que.push(std::make_shared<MsgNode>(msg, max_length));
//	if(pending){
//		return;
//	}
//	auto& msgnode = _send_que.front();
//	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len), 
//		std::bind(&CSession::handel_write, this, std::placeholders::_1, shared_from_this()));
//}

//对队列大小做限制
void CSession::Send(char* msg, int max_length) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << endl;
		return;
	}

	_send_que.push(make_shared<MsgNode>(msg, max_length));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::PrintRecvData(char* data, int length)
{
	std::stringstream ss;
	std::string result = "0x";
	for (int i = 0; i < length; i++) {
		std::string hexstr;
		ss << std::hex << std::setw(2) << std::setfill('0') << int(data[i]) << std::endl;
		ss >> hexstr;
		result += hexstr;
	}
	std::cout << "receive raw data is : " << result << std::endl;;
}

//监听对端发送消息
//void CSession::handel_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> _self_shared)
//{
//	if (!ec) {
//		std::cout << "Server receive data is:" << _data << std::endl;
//		Send(_data, bytes_transferred);
//		memset(_data, 0, MAX_LENGTH);
//		_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//			std::bind(&CSession::handel_read, this, std::placeholders::_1, std::placeholders::_2,  _self_shared));
//	}
//	else {
//		_server->ClearSession(_uuid);
//	}
//}


//解决粘包的handel_read
void CSession::handel_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> _self_shared){
	if(!ec){
		PrintRecvData(_data, bytes_transferred);
		std::chrono::milliseconds dura(2000);
		std::this_thread::sleep_for(dura);
		//已经移动的字符数
		int copy_len = 0;
		while(bytes_transferred > 0){
			if(!_b_head_parse){
				//收到的数据不足头部大小
				if(bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH){
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_head_node->_cur_len += bytes_transferred;
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&CSession::handel_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
				}
				//收到的数据比头部多
				//头部剩余未cpy的长度
				int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
				memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
				//更新已处理的data长度和剩余未处理的data长度
				copy_len += head_remain;
				bytes_transferred -= head_remain;
				//获取头部数据
				short data_len = 0;
				memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);
				//接受数据时，从网络字节序转本地字节序
				data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
				std::cout << "data_len is :" << data_len << std::endl;
				//头部长度非法
				if(data_len > MAX_LENGTH){
					std::cout << "invalid dat length is :" << data_len << std::endl;
					_server->ClearSession(_uuid);
					return;
				}
				_recv_msg_node = std::make_shared<MsgNode>(data_len);

				//消息的长度小于头部规定长度，说明数据未收全，则先将部分消息放到接受节点里
				if(bytes_transferred < data_len){
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_msg_node->_cur_len += bytes_transferred;
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
						bind(&CSession::handel_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
					_b_head_parse = true;
					return;
				}

				//若头部处理完成，缓冲区有足够填满整个消息的包
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, data_len);
				_recv_msg_node->_cur_len += data_len;
				copy_len += data_len;
				bytes_transferred -= data_len;
				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
				std::cout << "receive data is: " << _recv_msg_node->_data << std::endl;
				//send发送测试
				Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
				//当前包处理完成，继续轮询未处理数据
				_b_head_parse = false;
				_recv_head_node->Clear();
				if(bytes_transferred <= 0){
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						bind(&CSession::handel_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
					return;
				}
				continue;
			}

			//头部已处理完，但接受的数据仍不足，处理剩余未处理的
			int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
			if(bytes_transferred < remain_msg){
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
				_recv_msg_node->_cur_len += bytes_transferred;
				::memset(_data, 0, MAX_LENGTH);
				_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
					bind(&CSession::handel_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
				return;
			}
			memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
			_recv_msg_node->_cur_len += remain_msg;
			bytes_transferred -= remain_msg;
			copy_len += remain_msg;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			std::cout << "receive data is: " << _recv_msg_node->_data << std::endl;
			//send发送测试
			Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
			_b_head_parse = false;
			_recv_head_node->Clear();
			//若刚好接受一个包完，初始化继续
			if(bytes_transferred <= 0){
				::memset(_data, 0, MAX_LENGTH);
				_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
					bind(&CSession::handel_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
				return;
			}
			continue;
		}
	}
	else{
		std::cout << "handle read failed, error is " << ec.what() << std::endl;
		//Close();
		_server->ClearSession(_uuid);
	}
}


//handle_read将收到数据发送给对端，完成触发handle_write回调函数
void CSession::handel_write(const boost::system::error_code& ec, std::shared_ptr<CSession> _self_shared)
{
	if (!ec) {
		/*memset(_data, 0, max_length);
		_socket.async_read_some(boost::asio::buffer(_data, max_length), std::bind(&csession::handel_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));*/
		
		//应答式
		std::lock_guard<std::mutex> lock(_send_lock);
		_send_que.pop();
		if(!_send_que.empty()){
			auto& msgnode = _send_que.front();
			boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
				std::bind(&CSession::handel_write, this, std::placeholders::_1, _self_shared));
		}
	}
	else {
		std::cout << "handle write failed, error is " << ec.what() << std::endl;
		_server->ClearSession(_uuid);
	}
}


