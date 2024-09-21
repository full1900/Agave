//--------------------------------------------------------------------
//	Agave.hpp.
//	09/27/2022.				created.
//	09/18/2024.				last modified.
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
	inline auto resume_background(void)
	{
		return details::bg_awaitable_t{ };
	}


	//--------------------------------------------------------------------
	inline auto resume_foreground(void)
	{
		return details::fg_awaitable_t{ };
	}


	//--------------------------------------------------------------------
	inline auto get_cancellation_token(void)
	{
		return details::get_cancellation_token_t{ };
	}


    //--------------------------------------------------------------------
    inline auto get_progress_controller(void)
    {
        return details::get_progress_controller_t{ };
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
    //  *** types for progress reportering mechanism ***
	//--------------------------------------------------------------------
    template <typename P>
    using ProgressReporter = details::progress_reporter_base_t<P>;

    //--------------------------------------------------------------------
    template <typename P>
    using ProgressController = details::progress_controller_t<P>;


	//--------------------------------------------------------------------
	//	*** token for cancellation ***
	//--------------------------------------------------------------------
	template <typename Promise>
	class CancellationToken
	{
		//--------------------------------------------------------------------
		//	friend class types.
		//--------------------------------------------------------------------
		template <typename>
		friend class details::cancellation_token_t;

		//--------------------------------------------------------------------

	public:
		//--------------------------------------------------------------------
		operator bool() const noexcept
		{
			return is_canceled();
		}

		//--------------------------------------------------------------------
		bool is_canceled(void) const noexcept
		{
			if (_promise)
				return _promise->_async_data->_is_cancel.load(std::memory_order::acquire);
            
			return false;
		}

		//--------------------------------------------------------------------
		bool enable_propagation(bool value = true) const noexcept
		{
			return _promise->enable_cancellation_propagation(value);
		}

		//--------------------------------------------------------------------

	private:
		CancellationToken(Promise* promise) noexcept : _promise{ promise }
		{
			//
		}

		//--------------------------------------------------------------------

	private:
		Promise*                    _promise;

	};


	//--------------------------------------------------------------------


}


//--------------------------------------------------------------------
#endif // !_AGAVE_HPP__






