//--------------------------------------------------------------------
//	demo3.cpp.
//	09/21/2024.				created.
//	09/21/2024.				last modified.
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
agave::AsyncAction
do_work_async(void)
{
	co_await agave::resume_background();
	auto token = co_await agave::get_cancellation_token();

	for (int i = 1; i <= 10; ++i)
	{
		std::cout << "* working: " << i << std::endl;

		if (token) // test whether this task has been canceled.
		{
			std::cout << "* work aborted!" << std::endl;
			co_return;
		}

		co_await 1s;
	}
	
	std::cout << "* work finished!" << std::endl;
}


//--------------------------------------------------------------------
agave::AsyncAction
foo(bool is_enable_propagation = true)
{
	auto token = co_await agave::get_cancellation_token();
	token.enable_propagation(is_enable_propagation);

	co_await do_work_async();
}


//--------------------------------------------------------------------
int main(void)
{
	std::cout << "VVV - Enable Propagation - VVV" << std::endl;
	auto action = foo();
	std::this_thread::sleep_for(5s);
	action.cancel();
	action.get();

	std::cout << std::endl << "VVV - Disable Propagation - VVV" << std::endl;
	action = foo(false);
	std::this_thread::sleep_for(5s);
	action.cancel();
	action.get();

	return 0;
}


//--------------------------------------------------------------------





