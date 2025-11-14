//--------------------------------------------------------------------
//	Agave.hpp.
//	09/27/2022.				created.
//	11/11/2025.				last modified.
//--------------------------------------------------------------------
//	*	Agave(TM) Coroutine Framework (based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#pragma once

#ifndef _AGAVE_HPP__
#define _AGAVE_HPP__


//--------------------------------------------------------------------
//	headers...
//--------------------------------------------------------------------
#include "AgaveDetails.hpp"


//--------------------------------------------------------------------
namespace agave
{
	//--------------------------------------------------------------------
	inline void set_bg_entry(std::function<void(std::function<void(void)>)> bg_entry) noexcept
	{
		details::__BGThread = bg_entry;
	}

	//--------------------------------------------------------------------
	inline void set_job_entry(std::function<void(std::function<void(void)>)> job_entry) noexcept
	{
		details::__JobThread = job_entry;
	}

	//--------------------------------------------------------------------
	inline void set_fg_entry(std::function<void(std::function<void(void)>)> fg_entry) noexcept
	{
		details::__FGThread = fg_entry;
	}


	//--------------------------------------------------------------------
	//	*** token for cancellation ***
	//--------------------------------------------------------------------
	template <typename T>
	using CancellationToken = details::cancellation_token_t<T>;


	//--------------------------------------------------------------------
	template <typename T = void>
	inline decltype(auto) 
		suspend_always(
		std::function<void(std::coroutine_handle<>)> ready_cb = nullptr,
		std::function<T(void)> resume_cb = nullptr)
	{
		return details::suspend_always_t{ ready_cb, resume_cb };
	}


	//--------------------------------------------------------------------
	inline decltype(auto) resume_background(
		std::function<void(std::function<void(void)>)> current_entry = nullptr)
	{
		return details::bg_awaitable_t{ current_entry };
	}


	//--------------------------------------------------------------------
	inline decltype(auto) resume_foreground(
		std::function<void(std::function<void(void)>)> current_entry = nullptr)
	{
		return details::fg_awaitable_t{ current_entry };
	}


	//--------------------------------------------------------------------
	inline decltype(auto) get_cancellation_token(void)
	{
		return details::get_cancellation_token_awaiter_t{ };
	}


    //--------------------------------------------------------------------
    inline decltype(auto) get_progress_controller(void)
    {
        return details::get_progress_controller_t{ };
    }


	//--------------------------------------------------------------------
    inline decltype(auto) get_raw_coroutine_handle(void)
    {
        return details::get_raw_coroutine_handle_t{ };
    }


	//--------------------------------------------------------------------
	inline decltype(auto) get_caller_raw_coroutine_handle(void)
	{
		return details::get_caller_raw_coroutine_handle_t{ };
	}


    //--------------------------------------------------------------------
    //	*** returned objects for asynchronous functions ***
	//--------------------------------------------------------------------
	using AsyncAction = details::async_action_base_t<>;

    //--------------------------------------------------------------------
    template <typename P>
    using AsyncActionWithProgress = details::async_action_base_t<P>;

    //--------------------------------------------------------------------
    template <typename T>
    using AsyncOperation = details::async_operation_base_t<T>;

    //--------------------------------------------------------------------
	template <typename T, typename P>
	using AsyncOperationWithProgress = details::async_operation_base_t<T, P>;


    //--------------------------------------------------------------------
    //  *** types for progress reporting mechanism ***
	//--------------------------------------------------------------------
    template <typename P>
    using ProgressReporter = details::progress_reporter_base_t<P>;

    //--------------------------------------------------------------------
    template <typename P>
    using ProgressController = details::progress_controller_t<P>;


	//--------------------------------------------------------------------


}


//--------------------------------------------------------------------
#endif // !_AGAVE_HPP__






