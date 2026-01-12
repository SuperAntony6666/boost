#include <iostream>
#include <boost/asio.hpp>
#include "CSession.h"
#include "CServer.h"

int main()
{
	try
	{
		boost::asio::io_context ioc;
		CServer s(ioc, 6009);
		ioc.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception" << e.what() << std::endl;
	}

	return 0;
}


//小端序

//using namespace std;
//
//// 判断当前系统的字节序是大端序还是小端序
//bool is_big_endian() {
//	int num = 1;
//	if (*(char*)&num == 1) {
//		// 当前系统为小端序
//		return false;
//	}
//	else {
//		// 当前系统为大端序
//		return true;
//	}
//}
//
//int main() {
//	int num = 0x12345678;
//	char* p = (char*)&num;
//
//	cout << "原始数据：" << hex << num << endl;
//
//	if (is_big_endian()) {
//		cout << "当前系统为大端序" << endl;
//		cout << "字节序为：";
//		for (int i = 0; i < sizeof(num); i++) {
//			cout << hex << (int)*(p + i) << " ";
//		}
//		cout << endl;
//	}
//	else {
//		cout << "当前系统为小端序" << endl;
//		cout << "字节序为：";
//		for (int i = sizeof(num) - 1; i >= 0; i--) {
//			cout << hex << (int)*(p + i) << " ";
//		}
//		cout << endl;
//	}
//
//	return 0;
//}
// 