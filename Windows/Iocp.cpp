//--------------------------------------------------------------------
//	Iocp.cpp.
//	08/18/2025.				created.
//	08/21/2025.				last modified.
//--------------------------------------------------------------------
//	*	I/O Completion Ports (IOCP) on Windows module.
//	*	Agave(TM) Coroutine Framework (based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#include "Iocp.h"
#include <semaphore>
#include <unordered_map>
#include "../Agave.hpp"
#include "../Recycler.hpp"


//--------------------------------------------------------------------
namespace agave::win
{
	//--------------------------------------------------------------------
	typedef struct _IOCP_Entry
	{
		OVERLAPPED					ove{ 0 };
		std::coroutine_handle<>		h{ nullptr };
		unsigned long				bytes_transferred{ 0 };
		~_IOCP_Entry()
		{
			return;
		}

	} IOCP_Entry, * PIOCP_Entry;

	//--------------------------------------------------------------------
	std::unordered_map<HANDLE, std::vector<agave::AsyncAction>>		Handle2Actions;
	agave::Recycler<IOCP_Entry>										recycler{ 30 };

	//--------------------------------------------------------------------
	agave::AsyncAction run_task(HANDLE iocp);

	//--------------------------------------------------------------------

}


//--------------------------------------------------------------------
HANDLE 
agave::win::create_iocp(unsigned long thread_num /*= 0*/)
{
	auto iocp = ::CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		nullptr,
		0,
		thread_num);

	if (iocp == INVALID_HANDLE_VALUE)
		return nullptr;

	unsigned long long thread_quantity = !thread_num ? 
		std::thread::hardware_concurrency() * 2 : thread_num;

	Handle2Actions.emplace(iocp, std::vector<agave::AsyncAction>(thread_quantity));

	for (int i = 0; i < thread_quantity; ++i)
		Handle2Actions[iocp][i] = run_task(iocp);

	return iocp;
}


//--------------------------------------------------------------------
agave::AsyncOperation<long long> 
agave::win::resume_on_iocp(std::function<void(LPOVERLAPPED ove)> ove_consumer)
{
	std::coroutine_handle<> h = co_await agave::get_raw_coroutine_handle();

	std::shared_ptr<IOCP_Entry> entry{ recycler.create(), [](void*p)
		{
			recycler.recycle(reinterpret_cast<PIOCP_Entry>(p));
		} };

	entry->h = h;
	
	co_await agave::suspend_always([entry, ove_consumer](std::coroutine_handle<> h)
		{
			entry->ove.Internal = 0;
			entry->ove.InternalHigh = 0;
			entry->ove.Offset = 0;
			entry->ove.OffsetHigh = 0;
			entry->ove.Pointer = nullptr;
			ove_consumer(&entry->ove);
		});

	co_return entry->bytes_transferred;
}


//--------------------------------------------------------------------
bool 
agave::win::join_iocp(
	HANDLE work_handle, 
	HANDLE iocp)
{
	auto old_iocp = ::CreateIoCompletionPort(
		work_handle,
		iocp,
		reinterpret_cast<ULONG_PTR>(work_handle),
		0);

	if (!old_iocp)
		return false;

	return true;
}


//--------------------------------------------------------------------
agave::AsyncOperation<bool>
agave::win::close_iocp(HANDLE iocp)
{
	co_await agave::resume_background();

	auto it = Handle2Actions.find(iocp);
	if (it == Handle2Actions.end())
		co_return false;

	for (int i = 0; i < it->second.size(); ++i)
		::PostQueuedCompletionStatus(iocp, 0, 0, nullptr);
	
	for (auto&& action : it->second)
		action.get();

	Handle2Actions.erase(it);

	::CloseHandle(iocp);

	co_return true;
}


//--------------------------------------------------------------------
agave::AsyncOperation<bool>
agave::win::cleanup(void)
{
	co_await agave::resume_background();

	for (auto& v : Handle2Actions)
	{
		for (int i = 0; i < v.second.size(); ++i)
			::PostQueuedCompletionStatus(v.first, 0, 0, nullptr);

		for (auto&& action : v.second)
			action.get();

		::CloseHandle(v.first);
	}

	Handle2Actions.clear();

	auto data_list = recycler.pop_all();
	for (auto&& data : data_list)
	{
		if (data->ove.hEvent)
			::CloseHandle(data->ove.hEvent);
		delete data;
	}

	co_return true;
}


//--------------------------------------------------------------------
namespace agave::win
{
	//--------------------------------------------------------------------
	agave::AsyncAction run_task(HANDLE iocp)
	{
		co_await agave::resume_background();

		DWORD bytes_transferred{ 0 };
		HANDLE work_handle{ nullptr };
		PIOCP_Entry entry{ nullptr };

		while (true)
		{
			bool successes = ::GetQueuedCompletionStatus(
				iocp,
				&bytes_transferred,
				reinterpret_cast<PULONG_PTR>(&work_handle),
				reinterpret_cast<LPOVERLAPPED*>(&entry),
				INFINITE);

			if (!successes)
			{
				DWORD err = ::GetLastError();

				if (entry)
					entry->h.destroy();

				continue;
			}

			if (!entry && !work_handle)
				break;

			entry->bytes_transferred = bytes_transferred;

			entry->h.resume();
		}

	}

	//--------------------------------------------------------------------

}


//--------------------------------------------------------------------





