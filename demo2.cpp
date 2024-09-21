//--------------------------------------------------------------------
//	demo2.cpp.
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
	// switch to the (custom) background thread.
	co_await agave::resume_background();
	std::wcout << L"do works... on thread: " << std::this_thread::get_id() << std::endl;

	//std::this_thread::sleep_for(5s);
	co_await 5s; // wake up on the custom job thread.
	std::wcout << L"wake up... on thread: " << std::this_thread::get_id() << std::endl;
}


//--------------------------------------------------------------------
int main(void)
{
	// set the custom background thread / thread pool.
	agave::set_bg_entry([](std::function<void(void)> procedure)
		{ std::thread{ procedure }.detach(); });

	// set the custom job (co_await time duration) thread / thread pool.
	agave::set_job_entry([](std::function<void(void)> procedure)
		{ std::thread{ procedure }.detach(); });

	std::wcout << L"* main thread id: " << std::this_thread::get_id() << std::endl;
	do_work_async();

	for (int i = 0; i <= 10; ++i) // working.
	{
		std::this_thread::sleep_for(1s);
		std::wcout << L"* main thread working on: " << std::this_thread::get_id() << std::endl;
	}

	return 0;

}


//--------------------------------------------------------------------





