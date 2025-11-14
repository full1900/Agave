//--------------------------------------------------------------------
//	QuickThreads.cpp.
//	11/11/2025.				created.
//	11/13/2025.				last modified.
//--------------------------------------------------------------------
//	*	QuickThreads - A Part of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#include "QuickThreads.h"


//--------------------------------------------------------------------
agave::QuickThreads::QuickThreads() :
	_task_queue{ std::shared_ptr<Queue<ITask>>(new Queue<ITask>) }
{
	//
}


//-------------------------------------------------------------------- 
agave::QuickThreads::~QuickThreads()
{
	set_and_release_threads(true);

	if (_task_queue)
		while (auto task = _task_queue->pop())
			delete task;
	
}


//--------------------------------------------------------------------
bool 
agave::QuickThreads::add_task(ITask* task)
{
	if (!task || !_task_queue)
		return false;

	_task_queue->push(task);
	_cv.notify_one();

	return true;
}


//--------------------------------------------------------------------
std::shared_ptr<agave::ITask> 
agave::QuickThreads::join_and_get(void)
{
	while (true)
	{
		if (auto task = _task_queue->pop())
			return std::shared_ptr<ITask>{ task };

		std::unique_lock lck{ _mx };

		if (_is_exit)
			return nullptr;

		_cv.wait(lck);
		
		if (_is_exit)
			return nullptr;
	}
	
}


//--------------------------------------------------------------------
void 
agave::QuickThreads::set_and_release_threads(bool is_release /*= false*/)
{
	if (is_release)
	{
		_is_exit = true;
		std::unique_lock lck{ _mx };
		_cv.notify_all();
	}
	else
		_is_exit = false;

}


//--------------------------------------------------------------------





