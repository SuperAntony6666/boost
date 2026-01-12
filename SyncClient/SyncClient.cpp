#include <iostream>
#include <boost/asio.hpp>
#define HEAD_LENGTH 2
const int MAX_LENGTH = 1024;
//
//int main()
//{
//	try
//	{
//		//创建上下文服务
//		boost::asio::io_context ioc;
//
//		//构造endpoint
//		boost::asio::ip::tcp::endpoint remote_ep(boost::asio::ip::address::from_string("127.0.0.1"), 6009);
//		boost::asio::ip::tcp::socket sock(ioc);
//		//定义一个错误
//		boost::system::error_code error = boost::asio::error::host_not_found;
//		sock.connect(remote_ep, error);
//		if (error) {
//			std::cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
//			return 0;
//		}
//		std::cout << "Enter Message:";
//		char request[MAX_LENGTH];
//		//获取客户端输入
//		std::cin.getline(request, MAX_LENGTH);
//		std::size_t request_length = strlen(request);
//		boost::asio::write(sock, boost::asio::buffer(request, request_length));
//
//		char reply[MAX_LENGTH];
//		//read必须填满缓冲才会返回，这里会造成死等
//		//std::size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply, reply_length));
//		boost::system::error_code receive_error;
//		std::size_t reply_length = sock.read_some(boost::asio::buffer(reply, MAX_LENGTH), receive_error);
//		std::cout << "Reply is:";
//		std::cout.write(reply, reply_length);
//		std::cout << "\n";
//	}
//	catch (const std::exception& e)
//	{
//		std::cerr << "Exception: " << e.what() << std::endl;
//	}
//	return 0;
//}

//解决粘包,按消息长度，消息内容来发送
//int main()
//{
//	try {
//		//创建上下文服务
//		boost::asio::io_context   ioc;
//		//构造endpoint
//		boost::asio::ip::tcp::endpoint  remote_ep(boost::asio::ip::address::from_string("127.0.0.1"), 6009);
//		boost::asio::ip::tcp::socket  sock(ioc);
//		boost::system::error_code   error = boost::asio::error::host_not_found; ;
//		sock.connect(remote_ep, error);
//		if (error) {
//			std::cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
//			return 0;
//		}
//
//		std::cout << "Enter message: ";
//		char request[MAX_LENGTH];
//		std::cin.getline(request, MAX_LENGTH);
//		size_t request_length = strlen(request);
//		char send_data[MAX_LENGTH] = { 0 };
//		memcpy(send_data, &request_length, 2);
//		memcpy(send_data + 2, request, request_length);
//		boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 2));
//
//		char reply_head[HEAD_LENGTH];
//		size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
//		short msglen = 0;
//		memcpy(&msglen, reply_head, HEAD_LENGTH);
//		char msg[MAX_LENGTH] = { 0 };
//		size_t  msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));
//
//		std::cout << "Reply is: ";
//		std::cout.write(msg, msglen) << std::endl;
//		std::cout << "Reply len is " << msglen;
//		std::cout << "\n";
//	}
//	catch (std::exception& e) {
//		std::cerr << "Exception: " << e.what() << std::endl;
//	}
//	return 0;
//}


//收发分离+高频发送测试
int main()
{
	try {
		//创建上下文服务
		boost::asio::io_context   ioc;
		//构造endpoint
		boost::asio::ip::tcp::endpoint  remote_ep(boost::asio::ip::address::from_string("127.0.0.1"), 6009);
		boost::asio::ip::tcp::socket  sock(ioc);
		boost::system::error_code   error = boost::asio::error::host_not_found; ;
		sock.connect(remote_ep, error);
		if (error) {
			std::cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
			return 0;
		}

		std::thread send_thread([&sock] {
			for (;;) {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				const char* request = "hello world!";
				size_t request_length = strlen(request);
				char send_data[MAX_LENGTH] = { 0 };
				memcpy(send_data, &request_length, 2);
				memcpy(send_data + 2, request, request_length);
				boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 2));
			}
			});

		std::thread recv_thread([&sock] {
			for (;;) {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				std::cout << "begin to receive..." << std::endl;
				char reply_head[HEAD_LENGTH];
				size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
				short msglen = 0;
				memcpy(&msglen, reply_head, HEAD_LENGTH);
				char msg[MAX_LENGTH] = { 0 };
				size_t  msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));

				std::cout << "Reply is: ";
				std::cout.write(msg, msglen) << std::endl;
				std::cout << "Reply len is " << msglen;
				std::cout << "\n";
			}
			});

		send_thread.join();
		recv_thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}