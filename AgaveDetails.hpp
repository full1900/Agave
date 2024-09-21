//--------------------------------------------------------------------
//	AgaveDetails.hpp.
//	09/27/2022.				created.
//	09/14/2024.				last modified.
//--------------------------------------------------------------------
//	*	Agave(TM) Coroutine Framework (based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#pragma once

#ifndef _AGAVE_DETAILS_HPP__
#define _AGAVE_DETAILS_HPP__


//--------------------------------------------------------------------
//	headers...
//--------------------------------------------------------------------
#include "BJobScheduler.h"

#include <coroutine>
#include <stdexcept>
#include <chrono>
#include <type_traits>
#include <future>
#include <atomic>
#include <condition_variable>


//--------------------------------------------------------------------
//	incomplete types.
//--------------------------------------------------------------------
namespace agave
{
	//--------------------------------------------------------------------
	template <typename Promise>
	class CancellationToken;

	//--------------------------------------------------------------------

}


//--------------------------------------------------------------------
namespace agave::details
{
	//--------------------------------------------------------------------
	//	background awaiter object.
	//--------------------------------------------------------------------
	class bg_awaitable_t
	{
	public:
		constexpr bool await_ready() const noexcept { return false; }

		void await_suspend(std::coroutine_handle<> h) const
		{
			if (details::__BGThread)
			{
				details::__BGThread([h] { h.resume(); });
			}
			else
			{
				std::thread([h] { h.resume(); }).detach();
			}

		}

		constexpr void await_resume() const noexcept {}

	};


	//--------------------------------------------------------------------
	//	foreground awaiter object.
	//--------------------------------------------------------------------
	class fg_awaitable_t
	{
	public:
		constexpr bool await_ready() const noexcept { return false; }

		void await_suspend(std::coroutine_handle<> h) const
		{
			if (details::__FGThread)
			{
				details::__FGThread([h] { h.resume(); });
			}
			else
				h.resume();

		}

		constexpr void await_resume() const noexcept {}

	};


	//--------------------------------------------------------------------
	//  used for internal only.
	//--------------------------------------------------------------------
	class async_action_data_t
	{
	public:
		std::coroutine_handle<>					_h;        // outer coroutine handle.
		std::mutex								_mx;
		std::condition_variable				_cv;
		std::atomic_bool						_is_cancel{ false };
		std::atomic_bool						_is_ready{ false };
		std::function<void(void)>				_cancel_fn;
		BJobToken								_cb_token{ nullptr };
		bool									_cancellation_propagation{ true };
		std::weak_ptr<async_action_data_t>			_next;

	};


	//--------------------------------------------------------------------
	//  used for internal only.
	//--------------------------------------------------------------------
	template <typename T>
	class async_operation_data_t : public async_action_data_t
	{
	public:
		T                        _val;

	};


	//--------------------------------------------------------------------
	//  Async Data type traits.
	//--------------------------------------------------------------------
	template <typename T>
	class AsyncDataTraits   // primary template.
	{
	public:
		using AsyncDataType = async_operation_data_t<T>;
	};

	//--------------------------------------------------------------------
	template <>
	class AsyncDataTraits<void> // template specializations.
	{
	public:
		using AsyncDataType = async_action_data_t;
	};

	//--------------------------------------------------------------------
	template <typename T = void>
	using AsyncDataType = AsyncDataTraits<T>::AsyncDataType;


	//--------------------------------------------------------------------
	//	standard time span awaiter object.
	//--------------------------------------------------------------------
	template <typename Promise>
	class timespan_awaiter_t
	{
	public:
		//--------------------------------------------------------------------
		timespan_awaiter_t(
			Promise* promise,
			std::chrono::high_resolution_clock::duration dur) noexcept :
			_promise{ promise }, _dur{ dur },
			_async_data{ std::make_shared<AsyncDataType<>>() }
		{
			return;
		}

		//--------------------------------------------------------------------
		timespan_awaiter_t(timespan_awaiter_t const& other) noexcept :
			_promise{ other._promise }, _dur{ other._dur }, _h{ other._h },
			_async_data{ other._async_data },
			_is_awake{ other._is_awake.load(std::memory_order::consume) }
		{
			return;
		}

		//--------------------------------------------------------------------
		timespan_awaiter_t(timespan_awaiter_t&& other) noexcept :
			_promise{ other._promise }, _dur{ other._dur }, _h{ other._h },
			_async_data{ other._async_data },
			_is_awake{ other._is_awake.load(std::memory_order::consume) }
		{
			return;
		}

		//--------------------------------------------------------------------
		~timespan_awaiter_t()
		{
			//
		}

		//--------------------------------------------------------------------
		constexpr bool await_ready() const noexcept
		{
			return false;
		}

		//--------------------------------------------------------------------
		void await_suspend(std::coroutine_handle<> h) noexcept
		{
			_h = h;
			callback(std::bind(&timespan_awaiter_t::on_wakeup, this));
		}

		//--------------------------------------------------------------------
		constexpr void await_resume() const noexcept
		{
			return;
		}

		//--------------------------------------------------------------------
		operator bool() const noexcept
		{
			return is_canceled();
		}

		//--------------------------------------------------------------------
		bool is_canceled(void) const noexcept
		{
			if (_promise)
				return _promise->_async_data->_is_cancel.load(std::memory_order::consume);

			return false;

		}

		//--------------------------------------------------------------------
		void callback(std::function<void(void)> cancel_fn) const noexcept
		{
			auto job_tok = details::BJobScheduler::instance_ptr()->add_job(_dur, cancel_fn);
			_promise->cancellation_callback(cancel_fn, job_tok);
		}

		//--------------------------------------------------------------------


	private:
		//--------------------------------------------------------------------
		void on_wakeup(void)
		{
			if (_is_awake)
				return;

			_is_awake.store(true, std::memory_order::release);

			_h.resume();

		}

		//--------------------------------------------------------------------

	public:
		//--------------------------------------------------------------------
		Promise*                                        _promise;
		std::chrono::high_resolution_clock::duration	_dur;
		std::coroutine_handle<>							_h;
		std::shared_ptr<AsyncDataType<>>		        _async_data;
		std::atomic<bool>								_is_awake{ false };

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//  dummy type for cancellation mechanism.
	//--------------------------------------------------------------------
	struct get_cancellation_token_t {};


	//--------------------------------------------------------------------
	//	token for cancellation.
	//--------------------------------------------------------------------
	template <typename Promise>
	class cancellation_token_t
	{
	public:
		//--------------------------------------------------------------------
		cancellation_token_t(Promise* promise) noexcept : _promise{ promise }
		{
			//
		}

		//--------------------------------------------------------------------
		constexpr bool await_ready() const noexcept
		{
			return true;
		}

		//--------------------------------------------------------------------
		constexpr void await_suspend(std::coroutine_handle<>) const noexcept
		{
			//
		}

		//--------------------------------------------------------------------
		agave::CancellationToken<Promise> await_resume() const noexcept
		{
			return _promise;
		}

		//--------------------------------------------------------------------

	private:
		//--------------------------------------------------------------------
		Promise*						_promise;

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//  progress data.
	//--------------------------------------------------------------------
	template <typename Progress>
	class progress_data_t
	{
	public:
		Progress							_progress;
		bool								_is_ready{ false };
		bool								_is_finished{ false };
		std::mutex							_access_mx;
		std::coroutine_handle<>				_h;

	};


	//--------------------------------------------------------------------
	//  forward declarations.
	//--------------------------------------------------------------------
	template <typename Progress>
	class progress_reporter_t;


	//--------------------------------------------------------------------
	//  progress reporter base.
	//--------------------------------------------------------------------
	template <typename Progress, typename Reporter = progress_reporter_t<Progress>>
	class progress_reporter_base_t
	{
		//--------------------------------------------------------------------
		//	friend class types.
		//--------------------------------------------------------------------
		template <typename>
		friend class progress_controller_t;

		//--------------------------------------------------------------------
		template <typename>
		friend class async_progress_base_t;

		//--------------------------------------------------------------------
		template <typename>
		friend class progress_reporter_t;

		//--------------------------------------------------------------------

	public:
		//--------------------------------------------------------------------
		bool has_next(void) const
		{
			if (decltype(auto) reporter = static_cast<Reporter const*>(this))
				return reporter->has_next();

			return false;

		}

		//--------------------------------------------------------------------
		operator bool() const
		{
			return has_next();
		}

		//--------------------------------------------------------------------

	protected:
		//--------------------------------------------------------------------
		progress_reporter_base_t(std::shared_ptr<progress_data_t<Progress>> pg_data) :
			_pg_data{ pg_data }
		{
			//
		}

		//--------------------------------------------------------------------
		progress_reporter_base_t(void)
		{
			//
		}

		//--------------------------------------------------------------------


	protected:
		//--------------------------------------------------------------------
		std::shared_ptr<progress_data_t<Progress>>              _pg_data;

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//  progress reporter.
	//--------------------------------------------------------------------
	template <typename Progress>
	class progress_reporter_t : public progress_reporter_base_t<Progress, progress_reporter_t<Progress>>
	{
	public:
		//--------------------------------------------------------------------
		progress_reporter_t(std::shared_ptr<progress_data_t<Progress>> pg_data) noexcept
		{
			this->_pg_data = pg_data;
		}

		//--------------------------------------------------------------------
		progress_reporter_t(progress_reporter_base_t<Progress, progress_reporter_t<Progress>> const& other)
		{
			this->_pg_data = other._pg_data;
		}

		//--------------------------------------------------------------------
		constexpr bool await_ready() const noexcept
		{
			std::lock_guard lck(this->_pg_data->_access_mx);

			return this->_pg_data->_is_ready || this->_pg_data->_is_finished;
		}

		//--------------------------------------------------------------------
		constexpr void await_suspend(std::coroutine_handle<> h) const noexcept
		{
			std::lock_guard lck{ this->_pg_data->_access_mx };
			this->_pg_data->_h = h;
		}

		//--------------------------------------------------------------------
		Progress& await_resume() const noexcept
		{
			std::lock_guard lck(this->_pg_data->_access_mx);
			this->_pg_data->_is_ready = false;
			this->_pg_data->_h = nullptr;

			return this->_pg_data->_progress;

		}

		//--------------------------------------------------------------------
		bool has_next(void) const
		{
			std::lock_guard lck(this->_pg_data->_access_mx);
			return this->_pg_data->_is_ready || !this->_pg_data->_is_finished;
		}


		//--------------------------------------------------------------------

	};

    
    //--------------------------------------------------------------------
    //  progress controller.
    //--------------------------------------------------------------------
    template <typename Progress>
    class progress_controller_t
    {
    public:
        //--------------------------------------------------------------------
        progress_controller_t(std::shared_ptr<progress_data_t<Progress>> pt_data) noexcept :
            _pg_data(pt_data)
        {
            //
        };

        //--------------------------------------------------------------------
        void report_progress(Progress&& progress, bool is_finished = false) noexcept
        {
			std::unique_lock lck{ _pg_data->_access_mx };

			_pg_data->_is_ready = true;
            _pg_data->_is_finished = is_finished;
            _pg_data->_progress = std::move(progress);

			if (_pg_data->_h)
			{
				lck.unlock();
				_pg_data->_h();
			}
            
            
        }

		//--------------------------------------------------------------------
		void report_progress(Progress const& progress, bool is_finished = false)
		{
			std::unique_lock lck{ _pg_data->_access_mx };

			_pg_data->_is_ready = true;
			_pg_data->_is_finished = is_finished;
			_pg_data->_progress = progress;

			if (_pg_data->_h)
			{
				lck.unlock();
				_pg_data->_h();
			}

		}

        //--------------------------------------------------------------------
        void finish(bool is_finished = true)
        {
			std::lock_guard lck{ _pg_data->_access_mx };

            _pg_data->_is_finished = is_finished;
        }

        //--------------------------------------------------------------------

    private:
        //--------------------------------------------------------------------
        std::shared_ptr<progress_data_t<Progress>>              _pg_data;

        //--------------------------------------------------------------------

    };


	//--------------------------------------------------------------------
	//  dummy type for progress reporting mechanism.
	//--------------------------------------------------------------------
	struct get_progress_controller_t {};


	//--------------------------------------------------------------------
	//	awaiter for progress controller.
	//--------------------------------------------------------------------
	template <typename Progress>
	class progress_controller_awaiter_t
	{
	public:
		//--------------------------------------------------------------------
		progress_controller_awaiter_t(std::shared_ptr<progress_data_t<Progress>> pg_data) noexcept :
			_pg_data{ pg_data }
		{
			//
		}

		//--------------------------------------------------------------------
		constexpr bool await_ready() const noexcept
		{
			return true;
		}

		//--------------------------------------------------------------------
		constexpr void await_suspend(std::coroutine_handle<>) const noexcept
		{
			//
		}

		//--------------------------------------------------------------------
		progress_controller_t<Progress> await_resume() const noexcept
		{
			return _pg_data;
		}

		//--------------------------------------------------------------------

	private:
		//--------------------------------------------------------------------
		std::shared_ptr<progress_data_t<Progress>>              _pg_data;

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//  the base class of async action / async operation.
	//--------------------------------------------------------------------
	template <typename Progress>
	class async_progress_base_t
	{
        //--------------------------------------------------------------------
        //  friend class types.
        //--------------------------------------------------------------------
        template <typename, typename, typename>
        friend class promise_base_t;
        
        //--------------------------------------------------------------------

	public:
		//--------------------------------------------------------------------
		async_progress_base_t() : _pg_data{ new progress_data_t<Progress> }
		{
			//
		}

		//--------------------------------------------------------------------
		progress_reporter_base_t<Progress> get_progress_reporter(void)
		{
			return progress_reporter_base_t<Progress>(_pg_data);
		}

		//--------------------------------------------------------------------

	protected:
		//--------------------------------------------------------------------
		std::shared_ptr<progress_data_t<Progress>>              _pg_data;

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//  specializations for async_progress_base_t class.
	//--------------------------------------------------------------------
	template <>
	class async_progress_base_t<void>
	{
	public:
		//

	};

	//--------------------------------------------------------------------


	//--------------------------------------------------------------------
	//	declarations.
	//--------------------------------------------------------------------
	template <typename Progress> class async_action_t;
	template <typename Progress> class async_action_promise_t;
	template <typename T, typename P> class async_operation_t;
	template <typename T, typename P> class async_operation_promise_t;


	//--------------------------------------------------------------------
	//	*** for asynchronous function with no returned value ***
	//--------------------------------------------------------------------
	template <typename Progress = void, typename Action = async_action_t<Progress>>
	class async_action_base_t : public async_progress_base_t<Progress>
	{
		//--------------------------------------------------------------------
		//	friend class types.
		//--------------------------------------------------------------------
		template <typename, typename, typename>
		friend class promise_base_t;

		//--------------------------------------------------------------------
		template <typename>
		friend class async_action_t;

		//--------------------------------------------------------------------

	public:
		//--------------------------------------------------------------------
		void get(void)
		{
			if (auto action = static_cast<Action*>(this))
				return action->get();
            
            throw std::runtime_error("Agave: bad inherited type.");
		}

		//--------------------------------------------------------------------
		void cancel(void)
		{
			if (auto action = static_cast<Action*>(this))
				return action->cancel();
            
            throw std::runtime_error("Agave: bad inherited type.");
		}

		//--------------------------------------------------------------------
		using promise_type = async_action_promise_t<Progress>;

		//--------------------------------------------------------------------

	protected:
		//--------------------------------------------------------------------
		std::shared_ptr<AsyncDataType<>>		_async_data;
		std::coroutine_handle<>					_h;             // current coroutine handle.

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//	implementations for async_action_base_t.
	//--------------------------------------------------------------------
	template <typename Progress = void>
	class async_action_t : public async_action_base_t<Progress, async_action_t<Progress>>
	{
	public:
		//--------------------------------------------------------------------
		async_action_t(async_action_t const& other)
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}

		//--------------------------------------------------------------------
		async_action_t(async_action_t&& other) noexcept
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}

		//--------------------------------------------------------------------
		async_action_t(async_action_base_t<Progress, async_action_t<Progress>> const& other)
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}

		//--------------------------------------------------------------------
		async_action_t(async_action_base_t<Progress, async_action_t<Progress>>&& other) noexcept
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}


		//--------------------------------------------------------------------
		//  methods for outer awaiter.
		//--------------------------------------------------------------------
		bool await_ready() const noexcept
		{
			return this->_async_data->_is_ready;
		}

		//--------------------------------------------------------------------
		void await_suspend(std::coroutine_handle<> h) noexcept
		{
			this->_async_data->_h = h;
		}

		//--------------------------------------------------------------------
		constexpr void await_resume() const noexcept
        {
            //
        }

		//--------------------------------------------------------------------
		void get(void)
		{
			if (this->_async_data->_is_ready)
			{
				return;
			}
			else
			{
				std::unique_lock lck{ this->_async_data->_mx };
				if (this->_async_data->_is_ready)	// check again!
					return;
				this->_async_data->_cv.wait(lck);
			}

		}

		//--------------------------------------------------------------------
		void cancel(void)
		{
			this->_async_data->_is_cancel.store(true, std::memory_order::release);

			if (this->_async_data->_cancellation_propagation)
			{
				auto async_data = this->_async_data->_next.lock();
				while (async_data)
				{
					async_data->_is_cancel.store(true, std::memory_order::release);
					if (async_data->_cancel_fn)
					{
						async_data->_cancel_fn();
						details::BJobScheduler::instance_ptr()->remove_job(async_data->_cb_token);
					}


					if (!async_data->_cancellation_propagation)
						break;
					async_data = async_data->_next.lock();
				}

			}

		}

		//--------------------------------------------------------------------
		async_action_t(
			std::coroutine_handle<> const& h,
			std::shared_ptr<AsyncDataType<>> async_data)
		{
			this->_async_data = async_data;
			this->_h = h;

		}

		//--------------------------------------------------------------------


	};


	//--------------------------------------------------------------------
	//  *** for asynchronous function with returned value of T ***
	//--------------------------------------------------------------------
	template <typename T, typename Progress = void, typename Operation = async_operation_t<T, Progress>>
		requires(!std::is_void_v<T>)
	class async_operation_base_t : public async_progress_base_t<Progress>
	{
		//--------------------------------------------------------------------
		//  friend class types.
		//--------------------------------------------------------------------
		template <typename, typename, typename>
		friend class promise_base_t;

		//--------------------------------------------------------------------
		template <typename, typename>
		friend class async_operation_t;

		//--------------------------------------------------------------------

	public:
		//--------------------------------------------------------------------
		T& get(void)
		{
			if (auto operation = static_cast<Operation*>(this))
				return operation->get();
            
            throw std::runtime_error("Agave: bad inherited type.");
		}

		//--------------------------------------------------------------------
		void cancel(void)
		{
			if (auto operation = static_cast<Operation*>(this))
				return operation->cancel();
            
            throw std::runtime_error("Agave: bad inherited type.");
		}

		//--------------------------------------------------------------------
		using promise_type = async_operation_promise_t<T, Progress>;


		//--------------------------------------------------------------------

	protected:
		//--------------------------------------------------------------------
		std::shared_ptr<AsyncDataType<T>>				_async_data;
		std::coroutine_handle<>                         _h;

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//	implementations for async_operation_base_t.
	//--------------------------------------------------------------------
	template <typename T, typename Progress = void>
	class async_operation_t : 
		public async_operation_base_t<T, Progress, async_operation_t<T, Progress>>
	{
	public:
		//--------------------------------------------------------------------
		async_operation_t(async_operation_base_t<T, Progress, async_operation_t<T, Progress>> const& other)
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}

		//--------------------------------------------------------------------
		async_operation_t(async_operation_base_t<T, Progress, async_operation_t<T, Progress>>&& other) noexcept
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}

		//--------------------------------------------------------------------
		async_operation_t(async_operation_t const& other)
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}

		//--------------------------------------------------------------------
		async_operation_t(async_operation_t&& other) noexcept
		{
			this->_async_data = other._async_data;
			this->_h = other._h;
		}


		//--------------------------------------------------------------------
		//	methods for outer awaiter.
		//--------------------------------------------------------------------
		bool await_ready() const noexcept
		{
			return this->_async_data->_is_ready;
		}

		//--------------------------------------------------------------------
		void await_suspend(std::coroutine_handle<> h) noexcept
		{
			this->_async_data->_h = h;
		}

		//--------------------------------------------------------------------
		T& await_resume() const noexcept
		{
			return this->_async_data->_val;
		}

		//--------------------------------------------------------------------
		T& get(void)
		{
			if (this->_async_data->_is_ready)
			{
				return this->_async_data->_val;
			}
			else
			{
				std::unique_lock lck{ this->_async_data->_mx };
				if (this->_async_data->_is_ready)		// check again!
					return this->_async_data->_val;

				this->_async_data->_cv.wait(lck);
				return this->_async_data->_val;
			}

		}

		//--------------------------------------------------------------------
		void cancel(void)
		{
			this->_async_data->_is_cancel.store(true, std::memory_order::release);

			if (this->_async_data->_cancellation_propagation)
			{
				auto async_data = this->_async_data->_next.lock();
				while (async_data)
				{
					async_data->_is_cancel.store(true, std::memory_order::release);
					if (async_data->_cancel_fn)
					{
						async_data->_cancel_fn();
						details::BJobScheduler::instance_ptr()->remove_job(async_data->_cb_token);
					}


					if (!async_data->_cancellation_propagation)
						break;
					async_data = async_data->_next.lock();
				}

			}

		}

		//--------------------------------------------------------------------
		async_operation_t(
			std::coroutine_handle<> const& h,
			std::shared_ptr<AsyncDataType<T>> async_data)
		{
			this->_async_data = async_data;
			this->_h = h;
		}


		//--------------------------------------------------------------------


	};

    
	//--------------------------------------------------------------------
	//  the base class of promise for async action / operation.
	//--------------------------------------------------------------------
	template <typename T, typename Promise, typename Progress>
	class promise_base_t
	{
	public:
        //--------------------------------------------------------------------
        progress_controller_awaiter_t<Progress>
            await_transform(get_progress_controller_t t) noexcept
        {
            return _pg_data;
        }

        //--------------------------------------------------------------------
        void init_progress(async_progress_base_t<Progress>* progress) noexcept
        {
            if (progress)
                _pg_data = progress->_pg_data;
        }
        
		//--------------------------------------------------------------------
		cancellation_token_t<Promise>
			await_transform(get_cancellation_token_t t) noexcept
		{
			return { static_cast<Promise*>(this) };
		}
        
		//--------------------------------------------------------------------
		timespan_awaiter_t<Promise>
			await_transform(std::chrono::high_resolution_clock::duration t) noexcept
		{
			timespan_awaiter_t<Promise> time_span_awaiter{ static_cast<Promise*>(this), t };
			_async_data->_next = time_span_awaiter._async_data;

			return time_span_awaiter;
		}

		//--------------------------------------------------------------------
		auto await_transform(bg_awaitable_t&& awaiter) noexcept
		{
			return awaiter;
		}

		//--------------------------------------------------------------------
		auto& await_transform(bg_awaitable_t const& awaiter)
		{
			return awaiter;
		}

		//--------------------------------------------------------------------
		auto await_transform(fg_awaitable_t&& awaiter) noexcept
		{
			return awaiter;
		}

		//--------------------------------------------------------------------
		auto& await_transform(fg_awaitable_t const& awaiter)
		{
			return awaiter;
		}

		//--------------------------------------------------------------------
		template <typename P>
		auto await_transform(async_action_base_t<P>&& awaiter) noexcept
		{
			_async_data->_next = awaiter._async_data;
			return awaiter;
		}

		//--------------------------------------------------------------------
		template <typename P>
		auto& await_transform(async_action_base_t<P> const& awaiter)
		{
			_async_data->_next = awaiter._async_data;
			return awaiter;
		}

		//--------------------------------------------------------------------
		template <typename U, typename P>
		auto await_transform(async_operation_base_t<U, P, async_operation_t<U, P>>&& awaiter) noexcept
		{
			_async_data->_next = awaiter._async_data;
			return awaiter;
		}

		//--------------------------------------------------------------------
		template <typename U, typename P>
		auto& await_transform(async_operation_base_t<U, P, async_operation_t<U, P>> const& awaiter)
		{
			_async_data->_next = awaiter._async_data;
			return awaiter;
		}

		//--------------------------------------------------------------------
		template <typename P>
		auto await_transform(progress_reporter_base_t<P>&& awaiter) noexcept
		{
			return awaiter;
		}

		//--------------------------------------------------------------------
		template <typename P>
		auto& await_transform(progress_reporter_base_t<P> const& awaiter)
		{
			return awaiter;
		}

		//--------------------------------------------------------------------
		std::shared_ptr<AsyncDataType<T>>                       _async_data;
        std::shared_ptr<progress_data_t<Progress>>              _pg_data;

		//--------------------------------------------------------------------

	};


	//--------------------------------------------------------------------
	//  specializations for promise_base_t class.
    //--------------------------------------------------------------------
    template <typename T, typename Promise>
    class promise_base_t<T, Promise, void>
    {
    public:
        //--------------------------------------------------------------------
        void init_progress(async_progress_base_t<void>* progress) noexcept
        {
            // none
        }
        
        //--------------------------------------------------------------------
        cancellation_token_t<Promise>
            await_transform(get_cancellation_token_t t) noexcept
        {
            return { static_cast<Promise*>(this) };
        }
        
        //--------------------------------------------------------------------
        timespan_awaiter_t<Promise>
            await_transform(std::chrono::high_resolution_clock::duration t) noexcept
        {
            timespan_awaiter_t<Promise> time_span_awaiter{ static_cast<Promise*>(this), t };
            _async_data->_next = time_span_awaiter._async_data;

            return time_span_awaiter;
        }

        //--------------------------------------------------------------------
        auto await_transform(bg_awaitable_t&& awaiter) noexcept
        {
            return awaiter;
        }

        //--------------------------------------------------------------------
        auto& await_transform(bg_awaitable_t const& awaiter)
        {
            return awaiter;
        }

        //--------------------------------------------------------------------
        auto await_transform(fg_awaitable_t&& awaiter) noexcept
        {
            return awaiter;
        }

        //--------------------------------------------------------------------
        auto& await_transform(fg_awaitable_t const& awaiter)
        {
            return awaiter;
        }

        //--------------------------------------------------------------------
		template <typename P>
        auto await_transform(async_action_base_t<P>&& awaiter) noexcept
        {
            _async_data->_next = awaiter._async_data;
            return awaiter;
        }

        //--------------------------------------------------------------------
		template <typename P>
        auto& await_transform(async_action_base_t<P> const& awaiter)
        {
            _async_data->_next = awaiter._async_data;
            return awaiter;
        }

        //--------------------------------------------------------------------
        template <typename U, typename P>
        auto await_transform(async_operation_base_t<U, P>&& awaiter) noexcept
        {
            _async_data->_next = awaiter._async_data;
            return awaiter;
        }

        //--------------------------------------------------------------------
		template <typename U, typename P>
        auto& await_transform(async_operation_base_t<U, P> const& awaiter)
        {
            _async_data->_next = awaiter._async_data;
            return awaiter;
        }

        //--------------------------------------------------------------------
        template <typename P>
        auto await_transform(progress_reporter_base_t<P>&& awaiter) noexcept
        {
            return awaiter;
        }

        //--------------------------------------------------------------------
        template <typename P>
        auto& await_transform(progress_reporter_base_t<P> const& awaiter)
        {
            return awaiter;
        }

        //--------------------------------------------------------------------
        template <typename U>
        auto& await_transform(std::future<U> const& future)
        {
            return future;
        }
        
        //--------------------------------------------------------------------
        template <typename U>
        auto await_transform(std::future<U>&& future) noexcept
        {
            return future;
        }
        
        //--------------------------------------------------------------------
        std::shared_ptr<AsyncDataType<T>>                       _async_data;

        //--------------------------------------------------------------------

    };


	//--------------------------------------------------------------------
	//	promise definition for async action.
	//--------------------------------------------------------------------
	template <typename Progress>
	class async_action_promise_t : 
		public promise_base_t<void, async_action_promise_t<Progress>, Progress>
	{
	public:
		//--------------------------------------------------------------------
		async_action_t<Progress> get_return_object(void)
		{
			this->_async_data = std::make_shared<AsyncDataType<>>();
            async_action_t<Progress> action = {
                std::coroutine_handle<async_action_promise_t>::from_promise(*this),
				this->_async_data };

			// initialize the progress data.
            this->init_progress(static_cast<async_progress_base_t<Progress>*>(&action));

            return action;
		}

		//--------------------------------------------------------------------
		constexpr std::suspend_never initial_suspend(void) const noexcept
		{
			return {};
		}

		//--------------------------------------------------------------------
		constexpr std::suspend_never final_suspend(void) const noexcept
		{
			return {};
		}

		//--------------------------------------------------------------------
		void return_void(void)
		{
			if (this->_async_data)
			{
				std::unique_lock lck{ this->_async_data->_mx };
				this->_async_data->_is_ready = true;

				// notify the current waiter.
				this->_async_data->_cv.notify_all();

				// resume the outer awaiter if it exists.
				if (this->_async_data->_h)
				{
					this->_async_data->_h.resume();
				}

			}

		}

		//--------------------------------------------------------------------
		void cancellation_callback(
			std::function<void(void)> cancel_fn,
			BJobToken tok)
		{
			this->_async_data->_cancel_fn = cancel_fn;
			this->_async_data->_cb_token = tok;
		}

		//--------------------------------------------------------------------
		bool enable_cancellation_propagation(bool val) const
		{
			auto ret = this->_async_data->_cancellation_propagation;
			this->_async_data->_cancellation_propagation = val;

			return ret;
		}

		//--------------------------------------------------------------------
		[[noreturn]]
		void unhandled_exception(void)
		{
			throw;
		}

		//--------------------------------------------------------------------
		std::coroutine_handle<>					_h;


		//--------------------------------------------------------------------


	};


	//--------------------------------------------------------------------
	//	promise definition for async operation.
	//--------------------------------------------------------------------
	template <typename T, typename Progress>
	class async_operation_promise_t : 
		public promise_base_t<T, async_operation_promise_t<T, Progress>, Progress>
	{
	public:
		//--------------------------------------------------------------------
		async_operation_t<T, Progress> get_return_object(void)
		{
			this->_async_data = std::make_shared<AsyncDataType<T>>();
			async_operation_t<T, Progress> action = {
				std::coroutine_handle<async_operation_promise_t>::from_promise(*this),
				this->_async_data };

			// initialize the progress data.
			this->init_progress(static_cast<async_progress_base_t<Progress>*>(&action));

			return action;

		}

		//--------------------------------------------------------------------
		constexpr std::suspend_never initial_suspend(void) const noexcept
		{
			return {};
		}

		//--------------------------------------------------------------------
		constexpr std::suspend_never final_suspend(void) const noexcept
		{
			return {};
		}

		//--------------------------------------------------------------------
		void return_value(T const& val) noexcept (std::is_nothrow_copy_constructible_v<T>)
		{
			if (this->_async_data)
			{
				// set the current value.
				this->_async_data->_val = val;

				std::unique_lock lck{ this->_async_data->_mx };
				this->_async_data->_is_ready = true;

				// notify the current waiter.
				this->_async_data->_cv.notify_all();

				// resume the outer awaiter if it exists.
				if (this->_async_data->_h)
				{
					this->_async_data->_h.resume();
				}

			}

		}

		//--------------------------------------------------------------------
		void return_value(T&& val) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			if (this->_async_data)
			{
				// set the current value.
				this->_async_data->_val = std::move(val);

				std::unique_lock lck{ this->_async_data->_mx };
				this->_async_data->_is_ready = true;

				// notify the current waiter.
				this->_async_data->_cv.notify_all();

				// resume the outer awaiter if it exists.
				if (this->_async_data->_h)
				{
					this->_async_data->_h.resume();
				}

			}
		}

		//--------------------------------------------------------------------
		void cancellation_callback(
			std::function<void(void)> cancel_fn,
			BJobToken tok)
		{
			this->_async_data->_cancel_fn = cancel_fn;
			this->_async_data->_cb_token = tok;
		}

		//--------------------------------------------------------------------
		bool enable_cancellation_propagation(bool val) const
		{
			auto ret = this->_async_data->_cancellation_propagation;
			this->_async_data->_cancellation_propagation = val;
			return ret;
		}

		//--------------------------------------------------------------------
		[[noreturn]]
		void unhandled_exception(void)
		{
			throw;
		}

		//--------------------------------------------------------------------
		std::coroutine_handle<>				    _h;


		//--------------------------------------------------------------------


	};


	//--------------------------------------------------------------------


} // end of 'agave::details'.


//--------------------------------------------------------------------
//  overload co_await for async action.
//--------------------------------------------------------------------
template <typename P>
inline
agave::details::async_action_t<P>
operator co_await(agave::details::async_action_base_t<P> const& async)
{
	return agave::details::async_action_t<P>{ async };
}


//--------------------------------------------------------------------
//  overload co_await for async operation.
//--------------------------------------------------------------------
template <typename T, typename P>
inline
agave::details::async_operation_t<T, P>
operator co_await(agave::details::async_operation_base_t<T, P> const& async)
{
	return agave::details::async_operation_t<T, P>{ async };
}


//--------------------------------------------------------------------
//  overload co_await for async progress reporter.
//--------------------------------------------------------------------
template <typename P>
inline
agave::details::progress_reporter_t<P>
operator co_await(agave::details::progress_reporter_base_t<P> const& async)
{
	return agave::details::progress_reporter_t<P>{ async };
}


//--------------------------------------------------------------------
//  overload co_await to allow co_await'ing std::future<T> and
//  std::future<void> by naively spawning a new thread for each
//  co_await.
//--------------------------------------------------------------------
template<typename T>
inline
auto 
operator co_await(std::future<T> future) noexcept
	requires(!std::is_reference_v<T>)
{
	//--------------------------------------------------------------------
	class fut_awaiter : public std::future<T>
	{
	public:
		//--------------------------------------------------------------------
		bool await_ready() const noexcept
		{
			using namespace std::chrono_literals;
            
			return this->wait_for(0ms) != std::future_status::timeout;
		}

		//--------------------------------------------------------------------
		void await_suspend(std::coroutine_handle<> h) const
		{
			std::thread([this, h]
				{
					this->wait();
					h();
				}).detach();
            
		}

		//--------------------------------------------------------------------
		T await_resume() { return this->get(); }

		//--------------------------------------------------------------------

	};

	//--------------------------------------------------------------------
	return fut_awaiter{ std::move(future) };

	//--------------------------------------------------------------------

}


//--------------------------------------------------------------------
#endif // !_AGAVE_DETAILS_HPP__







