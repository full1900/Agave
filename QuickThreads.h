//--------------------------------------------------------------------
//	QuickThreads.h.
//	11/11/2025.				created.
//	11/13/2025.				last modified.
//--------------------------------------------------------------------
//	*	QuickThreads - A Part of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#pragma once

#ifndef _QUICK_THREADS_H__
#define _QUICK_THREADS_H__


//--------------------------------------------------------------------
//	headers.
//--------------------------------------------------------------------
#include "LockFreeQueue.hpp"
#include <thread>
#include <condition_variable>


//--------------------------------------------------------------------
namespace agave
{
	//--------------------------------------------------------------------
	//	interface for task.
	//--------------------------------------------------------------------
	class ITask
	{
	public:
		virtual void run(void) = 0;
	};


	//--------------------------------------------------------------------
	class QuickThreads
	{
	public:
		QuickThreads();
		~QuickThreads();
		
		bool add_task(ITask* task);
		void set_and_release_threads(bool is_release = false);

		std::shared_ptr<ITask> join_and_get();

	private:
		std::shared_ptr<Queue<ITask>>			_task_queue;
		std::condition_variable					_cv;
		std::mutex								_mx;
		bool									_is_exit{ false };

	};


	//--------------------------------------------------------------------


}


//--------------------------------------------------------------------
#endif // !_QUICK_THREADS_H__





