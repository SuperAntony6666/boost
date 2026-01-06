#include "endpoint.h"
#include <iostream>
#include <boost/asio.hpp>
using namespace boost;

//客户端端点生成
int client_end_point() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 6009;
	
	boost::system::error_code ec;
	//由string解析ip
	asio::ip::address ip_address = asio::ip::address::from_string(raw_ip_address, ec);

	if (ec.value() != 0) {
		std::cout << "解析ip地址错误,错误代码=" << ec.value() << "。信息是：" << ec.message() << std::endl;
		return ec.value();
	}

	//生成端点
	asio::ip::tcp::endpoint ep(ip_address, port_num);

	return 0;
}

int server_end_point() {
	unsigned short port_num = 3333;
	//服务器可以直接生成地址,any允许任何人通讯
	asio::ip::address ip_address = asio::ip::address_v6::any();
	asio::ip::tcp::endpoint ep(ip_address, port_num);
}

int create_tcp_socket() {
	asio::io_context ioc;
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	asio::ip::tcp::socket sock(ioc);

	boost::system::error_code ec;

	sock.open(protocol, ec);
	if (ec.value() != 0) {
		std::cout << "open socket错误，错误代码=" << ec.value() << "。信息是：" << ec.message() << std::endl;
		return ec.value();
	}

	return 0;
}

int create_acceptor_socket() {
	asio::io_context ioc;
	/*asio::ip::tcp protocol = asio::ip::tcp::v6();
	asio::ip::tcp::acceptor acceptor(ioc);
	boost::system::error_code ec;

	acceptor.open(protocol, ec);
	if (ec.value() != 0) {
		std::cout << "acceptor socket错误，错误代码=" << ec.value() << "。信息是：" << ec.message() << std::endl;
		return ec.value();
	}*/

	//新写法，接收器名为acc，服务器端口6900接受ipv4的socket，换上面
	asio::ip::tcp::acceptor acc(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 6009));

	return 0;
}

int bind_acceptor_socket() {
	unsigned short port_num = 6009;
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ioc;
	asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
	boost::system::error_code ec;
	acceptor.bind(ep, ec);
	if (ec.value() != 0) {
		// Failed to bind the acceptor socket. Breaking
		// execution.
		std::cout << "Failed to bind the acceptor socket."
			<< "Error code = " << ec.value() << ". Message: "
			<< ec.message();

		return ec.value();
	}

	return 0;

}

int connect_to_end() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 6009;
	try
	{
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());

		sock.connect(ep);
	}
	catch (system::system_error e)
	{
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();

		return e.code().value();
	}

}

//DNS解析
int dns_connect_to_end() {
	std::string host = "www.baidu.com";
	std::string port_num = "6009";
	asio::io_context ioc;
	asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::numeric_service);
	asio::ip::tcp::resolver resolver(ioc);
	try
	{
		asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_query);
		asio::ip::tcp::socket sock(ioc);
		asio::connect(sock, it);
	}
	catch (system::system_error e)
	{
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();

		return e.code().value();
	}
}

//服务器接受连接
int accept_new_connect() {
	const int BACKLOG_SIZE = 30;

	unsigned short port_num = 6009;
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ioc;
	try
	{
		asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
		acceptor.bind(ep);
		acceptor.listen(BACKLOG_SIZE);
		asio::ip::tcp::socket sock(ioc);
		//阻塞
		acceptor.accept(sock);
	}
	catch (system::system_error e)
	{
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();

		return e.code().value();
	}

}

//str转buffer
void use_const_buffer() {
	std::string buf = "Hello world";
	asio::const_buffer asio_buf(buf.c_str(), buf.length());
	std::vector<asio::const_buffer> buffers_sequence;
	buffers_sequence.push_back(asio_buf);
}

//同上
void use_buffer_str() {

	asio::const_buffers_1 output_buf = asio::buffer("Hello world");
}

//字符数据缓冲区
void ust_buffer_array() {
	const std::size_t BUF_SIZE_BYTES = 20;
	std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);
	auto input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES);
}

//同步写，发送到操作系统的“内核发送缓冲区”，然后由操作系统负责发送到“对端”。
void write_to_socket(asio::ip::tcp::socket & sock) {
	std::string buf = "Hello World";
	//偏移量
	std::size_t total_bytes_written = 0;
	while (total_bytes_written != buf.length()) {
		//write_some（发送位置地址，剩余长度）,比如llo World,起始地址，剩余长度
		total_bytes_written += sock.write_some(asio::buffer(buf.c_str() + total_bytes_written, buf.length() - total_bytes_written));
	}
}

//同步写send，将buffer中的内容一次性发送给对端，若缓冲区满无法发送，则阻塞等待直到缓冲区可用
int send_data_by_send() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 6009;
	try
	{
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
		write_to_socket(sock);
		/*std::string buf = "Hello World";
		int send_length = sock.send(asio::buffer(buf.c_str(), buf.length()));
		if (send_length <= 0) {
			std::cout << "Send Failed" << std::endl;
			return 0;
		}*/

		//或者sock.send()一次性全部发送完
		std::string buf = "Hello World";
		//返回值小于0错误，=0对端关闭，>0返回length长度
		int send_length = sock.send(asio::buffer(buf.c_str(), buf.length()));

	}
	catch (system::system_error e)
	{
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();

		return e.code().value();
	}
	return 0;
}