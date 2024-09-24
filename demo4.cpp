//--------------------------------------------------------------------
//	demo4.cpp.
//	09/21/2024.				created.
//	09/22/2024.				last modified.
//--------------------------------------------------------------------
//	*	Demonstrations of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#include "Agave.hpp"
#include <iostream>
#include <iomanip>


//--------------------------------------------------------------------
using namespace std::chrono_literals;


//--------------------------------------------------------------------
agave::AsyncOperationWithProgress<long double, int>
do_work_async(void)
{
	co_await agave::resume_background();
	auto controller = co_await agave::get_progress_controller();

	for (int i = 0; i < 100; ++i)
	{
		controller.report_progress(i);
		co_await 100ms;
	}

	// set the progress with finish state.
	controller.report_progress(100, true); 
	//controller.finish(true);

	co_return 50.0l;
}


//--------------------------------------------------------------------
int main(void)
{
	auto operation = do_work_async();
	
	// lambda coroutine.
	[operation](void) mutable -> agave::AsyncAction
		{
			std::wstring backspaces /*= { 8, 8, 8, 8, 0 }*/;
			auto progress_reporter = operation.get_progress_reporter();
			while (progress_reporter) // test if the process is completed (has next report).
			{
				// resume on the operation's environment (thread).
				auto i = co_await progress_reporter; 

				// switch to foreground thread (UI) to report progress.
				co_await agave::resume_foreground();
                std::wcout << backspaces << std::right << std::setw(3) << i << L"%" << std::flush;
			}

			// switch to background to get the result.
			co_await agave::resume_background();
			auto v = co_await operation;

			// switch to foreground thread (UI) to display the result.
			co_await agave::resume_foreground();
			std::wcout << std::endl << L"Got the final result: " << v << std::endl;

		}().get();

	return 0;
}


//--------------------------------------------------------------------





