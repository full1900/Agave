//--------------------------------------------------------------------
//	demo5_win_iocp.cpp.
//	08/22/2025.				created.
//	11/06/2025.				last modified.
//--------------------------------------------------------------------
//	*	Demonstrations of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include "Agave.hpp"
#include "Windows/Iocp.h"


//--------------------------------------------------------------------
bool create_demo_test_file(void);
agave::AsyncAction read_file_async(HANDLE file);


//--------------------------------------------------------------------
int main(void)
{
	create_demo_test_file(); // create a demo file.
	auto iocp{ agave::win::create_iocp() };

	HANDLE file{ ::CreateFile(L"./iocp_test.txt", GENERIC_READ,
		FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr) };
	
	agave::win::join_iocp(file, iocp);
	read_file_async(file).get();

	::CloseHandle(file);
	file = nullptr;

	auto success{ agave::win::close_iocp(iocp).get() }; 
	// VVV clear all iocp handles and release resources. VVV
	success = agave::win::cleanup().get();

	return 0;
}


//--------------------------------------------------------------------
agave::AsyncAction read_file_async(HANDLE file)
{	
	char buf[100] { 0 };
	
	auto len{ co_await agave::win::resume_on_iocp([file, &buf](LPOVERLAPPED ove)
		{
			if (!ove->hEvent)
				ove->hEvent = ::CreateEvent(nullptr, true, false, nullptr);
			bool res = ::ReadFile(file, buf, 100, nullptr, ove);
		}) };
	
	if (len > 0)
		std::cout << buf << std::endl;
}


//--------------------------------------------------------------------
bool create_demo_test_file(void)
{
	std::ofstream fs{ L"./iocp_test.txt", std::ios_base::trunc };
	if (!fs)
		return false;

	fs.write("demonstration", 13);
	fs.close();
	if (!fs)
		return false;

	return true;
}


//--------------------------------------------------------------------





