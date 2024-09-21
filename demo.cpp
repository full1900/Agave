//--------------------------------------------------------------------
//	demo.cpp.
//	09/27/2022.				created.
//	09/20/2024.				last modified.
//--------------------------------------------------------------------
//	*	Demonstrations of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#include "Agave.hpp"
#include <iostream>


//--------------------------------------------------------------------
using namespace std::chrono_literals;


//--------------------------------------------------------------------
agave::AsyncOperation<int> foo(void);


//--------------------------------------------------------------------
int main(void)
{
	auto v = foo().get();           // classical blocked method.
    
	std::cout << "got the value on thread: " << std::this_thread::get_id() << std::endl;
	std::cout << v << std::endl;

	return 0;
}


//--------------------------------------------------------------------
agave::AsyncAction action_async(void)
{
	std::cout << "action started on thread: " << std::this_thread::get_id() << std::endl;
	co_await agave::resume_background();
	std::cout << "do action in 2s... on thread: " << std::this_thread::get_id() << std::endl;
	co_await 2s;
	std::cout << "do action in 2s done... on thread: " << std::this_thread::get_id() << std::endl;
}


//--------------------------------------------------------------------
agave::AsyncOperation<int> operation_async(void)
{
	std::cout << "operation started on thread: " << std::this_thread::get_id() << std::endl;
	co_await agave::resume_background();
	std::cout << "do operation in 3s... on thread: " << std::this_thread::get_id() << std::endl;
	co_await 3s;

	co_return 51;
}


//--------------------------------------------------------------------
agave::AsyncOperation<int> foo(void)
{
    co_await action_async();
    auto&& v = co_await operation_async();
    
    co_return v + 10;
}


//--------------------------------------------------------------------



