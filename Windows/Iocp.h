//--------------------------------------------------------------------
//	Iocp.h.
//	08/17/2025.				created.
//	08/18/2025.				last modified.
//--------------------------------------------------------------------
//	*	I/O Completion Ports (IOCP) on Windows module.
//	*	Agave(TM) Coroutine Framework (based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#pragma once

#ifndef _IOCP_H__
#define _IOCP_H__


//--------------------------------------------------------------------
//	headers...
//--------------------------------------------------------------------
#include <Windows.h>
#include <tuple>
#include "../Agave.hpp"


//--------------------------------------------------------------------
namespace agave::win
{
	//--------------------------------------------------------------------
	HANDLE create_iocp(unsigned long thread_num = 0ul);
	agave::AsyncOperation<bool> close_iocp(HANDLE iocp);
	agave::AsyncOperation<bool> cleanup(void);

	/*	[Return Type]:
		long long:	the size of the transferred data.	*/
	agave::AsyncOperation<long long>
		resume_on_iocp(std::function<void(LPOVERLAPPED ove)> ove_consumer);

	bool join_iocp(HANDLE work_handle, HANDLE iocp);

	//--------------------------------------------------------------------


}


//--------------------------------------------------------------------
#endif // !_IOCP_H__




